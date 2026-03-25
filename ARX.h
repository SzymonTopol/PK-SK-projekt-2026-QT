#ifndef ARX_H
#define ARX_H

#include <vector>
#include <deque>
#include <random>

struct Borders {

    double bottom;
    double top;
};

class ARX {
private:
    std::vector<double> A;  // wspolczynniki a0, a1, ..., an
    std::vector<double> B;  // wspolczynniki b0, b1, ..., bn
    int K;                  // rzad opoznienia
    double z = 0.0;         // zaklocenie

    std::mt19937 gen;
    std::normal_distribution<double> dist;

    std::deque<double> u;   // probki sygnalu wejsciowego
    std::deque<double> y;  // probki sygnalu wyjsciowego

    std::size_t max_y_size;
    std::size_t max_u_size;

    Borders borders_u = { -10.0, 10.0 };
    Borders borders_y = { -10.0, 10.0 };

    void insert_u(double value);
    void insert_y(double value);

public:
    ARX(const std::vector<double>& a, const std::vector<double>& b, int delay = 1, double noise = 0);

    // zmiana wektorw A i B i K
    void setA(const std::vector<double>& a);
    void setB(const std::vector<double>& b);
    void setK(int delay);
    void setZ(double noise_std_dev);

    // syg. wejsc i wyjsc
    void set_borders_y(Borders borders);
    void set_borders_u(Borders borders);

    // symulacja
    double simulate(double input_u);

    void reset();
};

#endif // ARX_H
