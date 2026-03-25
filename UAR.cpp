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

