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

    return y.back();
}

void ARX::reset() {
    u.assign(B.size() + K, 0.0);
    y.assign(A.size(), 0.0);
}

#include <QByteArray>
#include <cstring>
#include <cstdint>
#include <QApplication>

QByteArray ARX::serializeConfig() const {
    uint32_t countA = static_cast<uint32_t>(A.size());
    uint32_t countB = static_cast<uint32_t>(B.size());

    size_t sizeABytes = countA * sizeof(double);
    size_t sizeBBytes = countB * sizeof(double);

    // Całkowity rozmiar: pola proste + 2x rozmiar wektora (uint32_t) + bajty wektorów
    size_t totalSize = sizeof(K) + sizeof(z) + sizeof(borders_u) + sizeof(borders_y)
                       + sizeof(uint32_t) + sizeABytes
                       + sizeof(uint32_t) + sizeBBytes;

    QByteArray buf;
    buf.resize(totalSize);
    char* ptr = buf.data();

    memcpy(ptr, &K, sizeof(K)); ptr += sizeof(K);
    memcpy(ptr, &z, sizeof(z)); ptr += sizeof(z);
    memcpy(ptr, &borders_u, sizeof(borders_u)); ptr += sizeof(borders_u);
    memcpy(ptr, &borders_y, sizeof(borders_y)); ptr += sizeof(borders_y);

    memcpy(ptr, &countA, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    if(sizeABytes > 0) { memcpy(ptr, A.data(), sizeABytes); ptr += sizeABytes; }

    memcpy(ptr, &countB, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    if(sizeBBytes > 0) { memcpy(ptr, B.data(), sizeBBytes); ptr += sizeBBytes; }

    return buf;
}

void ARX::deserializeConfig(const QByteArray& buf) {
    qDebug()<<"Deserializacja";
    const char* ptr = buf.data();

    memcpy(&K, ptr, sizeof(K)); ptr += sizeof(K);
    memcpy(&z, ptr, sizeof(z)); ptr += sizeof(z);
    memcpy(&borders_u, ptr, sizeof(borders_u)); ptr += sizeof(borders_u);
    memcpy(&borders_y, ptr, sizeof(borders_y)); ptr += sizeof(borders_y);

    uint32_t countA = 0;
    memcpy(&countA, ptr, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    A.resize(countA);
    size_t sizeABytes = countA * sizeof(double);
    if(sizeABytes > 0) { memcpy(A.data(), ptr, sizeABytes); ptr += sizeABytes; }

    uint32_t countB = 0;
    memcpy(&countB, ptr, sizeof(uint32_t)); ptr += sizeof(uint32_t);
    B.resize(countB);
    size_t sizeBBytes = countB * sizeof(double);
    if(sizeBBytes > 0) { memcpy(B.data(), ptr, sizeBBytes); ptr += sizeBBytes; }

    qDebug()<<this->A[0];
}
