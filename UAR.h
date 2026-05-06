#ifndef UAR_H
#define UAR_H

#include "ARX.h"
#include "RegulatorPID.h"
#include "FunctionGenerator.h"
#include <memory>
#include <deque>

struct SimulationData {
    long long step; // krok
    double y; // wyjście
    double u; // sterowanie
    double e; // uchyb
    double w; // zadana
    RegulatorPID::last_pid_values pid_values;
};

// Struktura pomocnicza do przetrzymania danych na Kliencie w oczekiwaniu na paczkę
struct ClientTickData {
    double w;
    double e;
    double u;
    RegulatorPID::last_pid_values pid_values;
};

class UAR {
private:
    double run_step(double setpoint);

    std::unique_ptr<ARX> modelARX;
    std::unique_ptr<RegulatorPID> regulatorPID;
    std::unique_ptr<FunctionGenerator> functionGenerator;

    std::deque<SimulationData> output_history;
    const size_t max_output_history = 1000;

    long long step_count = 0;

public:
    UAR(const ARX &arx, const RegulatorPID &pid);
    UAR(const ARX &arx, const RegulatorPID &pid, const FunctionGenerator &fg);

    // --- TRYB STACJONARNY ---
    double simulate(double setpoint);
    double simulateWithGenerator();

    // --- TRYB SIECIOWY: KLIENT (Regulator) ---
    ClientTickData run_client_calc(double setpoint, double current_y);
    void commit_client_step(const ClientTickData& data, double y, long long step);

    // --- TRYB SIECIOWY: SERWER (Obiekt) ---
    double run_server_calc(double u);
    void commit_server_step(double w, double u, double y, long long step);

    // --- GETTERY I NARZĘDZIA ---
    long long getStepCount() const { return step_count; }
    void setStepCount(long long step) { step_count = step; }

    ARX& getARX();
    RegulatorPID& getRegulatorPID();
    FunctionGenerator& getFunctionGenerator();
    const std::deque<SimulationData>& getOutputHistory();
    void reset();
};

#endif // UAR_H
