#include "ARX.h"

ARX::ARX(const std::vector<double> &a, const std::vector<double> &b, int delay, double noise) {
    std::random_device rd;
    gen.seed(rd());

    setK(delay);
    setA(a);
    setB(b);
    setZ(noise);
}

void ARX::setA(const std::vector<double> &a) {
    this->A = a;
    y.resize(A.size(), 0.0);
}

void ARX::setB(const std::vector<double> &b) {
    this->B = b;
    u.resize(B.size() + K, 0.0);
}

void ARX::setK(int delay) {
    this->K = (delay < 1) ? 1 : delay;
}

void ARX::setZ(double noise_std_dev) {
    this->z = noise_std_dev;

    if (z > 0.0)
    {
        dist = std::normal_distribution<double>(0.0, z);
    }
}

void ARX::set_borders_y(Borders borders) {
    borders_y = borders;
}

void ARX::set_borders_u(Borders borders) {
    borders_u = borders;
}

void ARX::insert_u(double value) {
    if (value < borders_u.bottom) {
        value = borders_u.bottom;
    } else if (value > borders_u.top) {
        value = borders_u.top;
    }

    u.push_back(value);
}

void ARX::insert_y(double value) {
    if (value < borders_y.bottom) {
        value = borders_y.bottom;
    } else if (value > borders_y.top) {
        value = borders_y.top;
    }

    y.push_back(value);
}

double ARX::simulate(double input_u) {
    double new_y = 0.0;

    u.pop_front();
    insert_u(input_u);

    for (std::size_t j = 0; j < A.size(); j++) {
        new_y -= A[j] * y[y.size() - j - 1];
    }

    for (std::size_t j = 0; j < B.size(); j++) {
        new_y += B[j] * u[u.size() - j - K - 1];
    }

    if (z > 0.0)
    {
        new_y += dist(gen);
    }

    y.pop_front();
    insert_y(new_y);

    return new_y;
}

void ARX::reset() {
    u.assign(B.size() + K, 0.0);
    y.assign(A.size(), 0.0);
}
