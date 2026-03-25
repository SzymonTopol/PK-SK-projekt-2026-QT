#include "FunctionGenerator.h"
#define _USE_MATH_DEFINES
#include <cmath>

FunctionGenerator::FunctionGenerator(double amplitude, double T_RZ, int T_T, double steady) {
    set_amplitude(amplitude);
    set_real_period_and_interval(T_RZ, T_T);
    set_offset(steady);
    set_function_type(FunctionType::SIN);
    set_square_filling(0.5);
}

FunctionGenerator::FunctionGenerator(double amplitude, double T_RZ, int T_T, double steady, double fill_p) {
    set_amplitude(amplitude);
    set_real_period_and_interval(T_RZ, T_T);
    set_offset(steady);
    set_function_type(FunctionType::SQUARE);
    set_square_filling(fill_p);
}

void FunctionGenerator::set_amplitude(double amplitude) {
    if (amplitude < 0.0)
        amplitude = 0.0;

    this->_Amplitude = amplitude;
}

void FunctionGenerator::set_real_period_and_interval(double T_RZ, int T_T) {
    this->_T = static_cast<int>(round(T_RZ * 1000.0 / T_T));

    if (this->_T < 1)
        this->_T = 1;
}

void FunctionGenerator::set_square_filling(double p) {
    this->_p = p;

    if (this->_p < 0.0)
        this->_p = 0.0;
}

void FunctionGenerator::set_offset(double steady) {
    this->_Steady = steady;
}

void FunctionGenerator::set_function_type(FunctionType type) {
    this->_type = type;
}

double FunctionGenerator::sin_generator(unsigned int i) {
    return this->_Amplitude * sin((i % this->_T) / static_cast<double>(this->_T) * 2.0 * M_PI) + this->_Steady;
}

double FunctionGenerator::square_generator(unsigned int i) {
    if (i % this->_T < this->_p * this->_T)
        return this->_Amplitude + this->_Steady;
    else
        return this->_Steady;
}

double FunctionGenerator::get_value(unsigned int step_count) {
    switch (_type) {
    case FunctionType::SIN:
        return sin_generator(step_count);
    case FunctionType::SQUARE:
        return square_generator(step_count);
    default:
        return 0.0; // to do: obsługa błędu
    }
}
