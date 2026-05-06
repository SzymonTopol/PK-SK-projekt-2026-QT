#include "UAR.h"

UAR::UAR(const ARX &arx, const RegulatorPID &pid)
    : modelARX(std::make_unique<ARX>(arx)),
    regulatorPID(std::make_unique<RegulatorPID>(pid)) {
}

UAR::UAR(const ARX &arx, const RegulatorPID &pid, const FunctionGenerator &fg)
    : modelARX(std::make_unique<ARX>(arx)),
    regulatorPID(std::make_unique<RegulatorPID>(pid)),
    functionGenerator(std::make_unique<FunctionGenerator>(fg)) {
}

double UAR::simulate(double setpoint) {
    return run_step(setpoint);
}

double UAR::simulateWithGenerator() {
    if (!functionGenerator) {
        return 0.0;
    }

    double setpoint = functionGenerator->get_value(step_count);
    return run_step(setpoint);
}

double UAR::run_step(double setpoint) {
    double y_prev = 0.0;
    if (!output_history.empty()) {
        y_prev = output_history.back().y;
    }

    double e = setpoint - y_prev;
    double u = regulatorPID->simulate(e);
    double y = modelARX->simulate(u);

    output_history.push_back({step_count,y, u, e, setpoint, regulatorPID->getLastPidValues()});
    if (output_history.size() > max_output_history) {
        output_history.pop_front();
    }
    step_count++;

    return y;
}

//
// metody pod services manager tylko
//

ARX &UAR::getARX() {
    return *modelARX;
}

RegulatorPID &UAR::getRegulatorPID() {
    return *regulatorPID;
}

FunctionGenerator &UAR::getFunctionGenerator() {
    return *functionGenerator;
}

const std::deque<SimulationData> &UAR::getOutputHistory() {
    return output_history;
}

void UAR::reset() {
    if (modelARX) modelARX->reset();
    if (regulatorPID) regulatorPID->reset();
    output_history.clear();
    step_count = 0;
}

// ==========================================
// TRYB SIECIOWY - KLIENT (Regulator + Generator)
// ==========================================
ClientTickData UAR::run_client_calc(double setpoint, double current_y) {
    // Klient liczy uchyb i wyjście z PID, ale czeka na y od obiektu
    double e = setpoint - current_y;
    double u = regulatorPID->simulate(e);

    ClientTickData data;
    data.w = setpoint;
    data.e = e;
    data.u = u;
    data.pid_values = regulatorPID->getLastPidValues();
    return data;
}

void UAR::commit_client_step(const ClientTickData& data, double y, long long step) {
    output_history.push_back({step, y, data.u, data.e, data.w, data.pid_values});
    if (output_history.size() > max_output_history) {
        output_history.pop_front();
    }
    step_count = step + 1; // Aktualizujemy globalny krok
}

// ==========================================
// TRYB SIECIOWY - SERWER (Obiekt ARX)
// ==========================================
double UAR::run_server_calc(double u) {
    // Serwer ma za zadanie tylko wyliczyć równanie różnicowe ARX
    return modelARX->simulate(u);
}

void UAR::commit_server_step(double w, double u, double y, long long step) {
    // Serwer nie ma danych o e oraz PID (uzupełniamy 0 lub uproszczonym uchybem)
    double e = w - y;
    RegulatorPID::last_pid_values empty_pid = {0,0,0};

    output_history.push_back({step, y, u, e, w, empty_pid});
    if (output_history.size() > max_output_history) {
        output_history.pop_front();
    }
    step_count = step + 1;
}

