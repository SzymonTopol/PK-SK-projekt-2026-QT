#ifndef FUNCTIONGENERATOR_H
#define FUNCTIONGENERATOR_H

#include <math.h>
#include <QByteArray>

class FunctionGenerator {
public:
    enum class FunctionType {
        SIN,
        SQUARE
    };

    //SIN
    FunctionGenerator(double amplitude, double T_RZ, int T_T, double steady);
    //SQUARE
    FunctionGenerator(double amplitude, double T_RZ, int T_T, double steady, double fill_p);


    void set_amplitude(double amplitude);
    void set_real_period_and_interval(double T_RZ, int T_T);     // T_RZ w sekundach okres rzeczywisty, T_T w ms okres probek
    void set_square_filling(double p);                           // p - wypełnienie (0.0 - 1.0)
    void set_offset(double steady);

    void set_function_type(FunctionType type);

    double get_value(unsigned int step_count);                   // step_count - numer kroku symulacji

    //serializacja do sieci
    QByteArray serializeConfig() const;
    void deserializeConfig(const QByteArray& buf);

    // Gettery do synchronizacji stanu z ServicesManager
    double getAmplitude() const { return _Amplitude; }
    double getOffset() const { return _Steady; }
    double getSquareFilling() const { return _p; }
    FunctionType getType() const { return _type; }
    int getT() const { return _T; }

private:
    double _Amplitude = 1.0;        // amplituda
    double _Steady = 0.0;        // składowa stała (offset)
    int _T = 1;             // okres dyskretny (liczba próbek)
    double _p = 0.5;        // wypełnienie sygnału prostokątnego (0.0 - 1.0)

    FunctionType _type = FunctionType::SIN;

    double sin_generator(unsigned int i);
    double square_generator(unsigned int i);
};

#endif // FUNCTIONGENERATOR_H
