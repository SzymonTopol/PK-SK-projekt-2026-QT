#ifndef UAR_H
#define UAR_H

#include "ARX.h"
#include "RegulatorPID.h"
#include "FunctionGenerator.h"

//aby działało std:: (mega dziwne?)
#include <memory>

struct SimulationData {
    long long step; // krok
    double y; // wyjście
    double u; // sterowanie
    double e; // uchyb
    double w; // zadana
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

    int step_count = 0;

public:
    UAR(const ARX &arx, const RegulatorPID &pid);
    UAR(const ARX &arx, const RegulatorPID &pid, const FunctionGenerator &fg);

    double simulate(double setpoint);
    double simulateWithGenerator();

    ARX& getARX();
    RegulatorPID& getRegulatorPID();
    FunctionGenerator& getFunctionGenerator();
    const std::deque<SimulationData>& getOutputHistory();
    void reset();
};

#endif // UAR_H
