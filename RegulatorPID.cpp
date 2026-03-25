#include "RegulatorPID.h"

// Konstruktory:
RegulatorPID::RegulatorPID(double k, double Ti, double Td)
{
    this->k = k;
    this->Ti = Ti;
    this->Td = Td;
    sumError = 0.0;
    prevError = 0.0;
    liczCalk = LiczCalk::Zew;
}

// Metody liczące poszczególne składowe regulatora
double RegulatorPID::calcP(double e) const
{
    return k * e;
}

double RegulatorPID::calcI(double e)
{
    if (Ti == 0.0)
        return 0.0;

    double uI = 0.0;
    if (liczCalk == LiczCalk::Zew)
    {
        // (1/Ti) * Σ(ej)
        sumError += e;
        uI = sumError / Ti;
    }
    else
    {
        // Σ(ej/Ti)
        sumError = sumError + (e / Ti);
        uI = sumError;
    }
    return uI;
}

double RegulatorPID::calcD(double e)
{
    if (Td == 0.0)
        return 0.0;

    double uD = 0.0;
    uD = Td * (e - prevError);
    prevError = e;

    return uD;
}

// Metoda symulująca działanie regulatora PID:
double RegulatorPID::simulate(double e)
{
    _pidValues.p = calcP(e);
    _pidValues.i = calcI(e);
    _pidValues.d = calcD(e);

    return _pidValues.p + _pidValues.i + _pidValues.d;
}

const RegulatorPID::last_pid_values &RegulatorPID::getLastPidValues() const {
    return _pidValues;
}

void RegulatorPID::setK(double k) {
    this->k = k;
}

void RegulatorPID::setTi(double Ti) {
    this->Ti = Ti;
}

void RegulatorPID::setTd(double Td) {
    this->Td = Td;
}

// Metody ustawiające parametry regulatora:
void RegulatorPID::setLiczCalk(LiczCalk mode)
{
    if (liczCalk == mode)
        return; //brak zmiany

    if (mode == LiczCalk::Wew)
    {
        if (Ti != 0.0)
            sumError = sumError / Ti; //przeliczenie sumy uchybów
    }
    else //Zew
    {
        sumError = sumError * Ti; //przeliczenie sumy uchybów
    }
    liczCalk = mode;

    //wymaga tak dużo kodu pownieważ dla Zew: sumError przechowuje Σ(ej) (sumę uchybów),
    // a dla Wew: sumError przechowuje Ui (aktualną wartość członu całkującego, czyli Σ(ej/Ti))
}

void RegulatorPID::setStalaCalk(double Ti) {
    // to istnieje tylko dlatego zeby testy dzialaly
    setTi(Ti);
}

void RegulatorPID::resetIntegral()
{
    sumError = 0.0;
    prevError = 0.0;
}

// Metoda resetująca stan wewnętrzny regulatora
void RegulatorPID::reset()
{
    sumError = 0.0;
    prevError = 0.0;
}
