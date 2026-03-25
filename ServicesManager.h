#ifndef SERVICESMANAGER_H
#define SERVICESMANAGER_H

#include "UAR.h"
#include "saveloadmanager.h"

#include <QObject>
#include <QTimer>
#include <vector>
#include <QDebug>

class ServicesManager : public QObject {
    Q_OBJECT
public:
    static ServicesManager& getInstance();
    ServicesManager(ServicesManager const&) = delete;
    void operator=(ServicesManager const&) = delete;

    void startSimulation();
    void stopSimulation();
    void setSimulationInterval(int ms);
    bool isSimulationRunning() const;
    void setManualSetpoint(double val); // GUI musi przekazać aktualną wartość z inputa

    //
    // setery
    //
    void setArxParams(const std::vector<double>& a, const std::vector<double>& b, int delay, double noise = 0);
    void setArxBorders(const Borders border_u, const Borders border_y);
    void setPidParams(double p, double ti, double td, RegulatorPID::LiczCalk liczcalk);
    void setGeneratorParams(FunctionGenerator::FunctionType type, double amp, double period_s, int sample_ms, double offset, double fill);
    void setUseGenerator(bool use);

    //
    // gettery
    //
    const std::vector<double>& getArxA() const {return m_arx_a;};
    const std::vector<double>& getArxB() const {return m_arx_b;};
    int getArxDelay() const {return m_arx_delay;}
    double getArxNoise() const {return m_arx_noise;}
    Borders getBorderU() const {return m_border_u;};
    Borders getBorderY() const {return m_border_y;};


    double getPidP() const {return m_pid_p;};
    double getPidTi() const {return m_pid_ti;};
    double getPidTd() const {return m_pid_td;};
    RegulatorPID::LiczCalk getPidMethod() const {return m_LiczCalk;};

    bool getUseGenerator() const {return m_use_generator;};
    FunctionGenerator::FunctionType getGenType() const {return m_gen_type;};
    double getGenAmp() const {return m_gen_amplitude;};
    double getGenFreq() const {return m_gen_frequency;};
    double getGenOffset() const {return m_gen_offset;};
    double getGenfill() const {return m_gen_fill;};
    double getGenSampleMs() const {return m_gen_sample_ms;};


    //
    // kontrola
    //
    void resetPidIntegral();
    void applyParams();
    void runNextStep();
    void resetSimulation();
    void hardResetSimulation();

    //
    // historia do wykresu
    //
    const std::deque<SimulationData> &getSimulationData() const;

    //
    // Zapis i odczyt
    //
    bool saveState(const QString& filePath);
    bool loadState(const QString& filePath);
    QJsonArray vecToJson(const std::vector<double>& v);
    std::vector<double> jsonToVec(const QJsonArray& arr);

    //
    // Dla testów gui
    //
    void WypiszDane()
    {
        qDebug() << "=====ARX=====";
        qDebug() << "A: ";
        foreach (double var, m_arx_a) {
            qDebug() << var;
        }
        qDebug() << "B: ";
        foreach (double var, m_arx_b) {
            qDebug() << var;
        }
        qDebug() << "K: ";
        qDebug() << m_arx_delay;
        qDebug() << "Z: ";
        qDebug() << m_arx_noise;

        qDebug() << "=====PID=====";
        qDebug() << "P: ";
        qDebug() << m_pid_p;
        qDebug() << "Ti: ";
        qDebug() << m_pid_ti;
        qDebug() << "Td: ";
        qDebug() << m_pid_td;
        if (m_LiczCalk == RegulatorPID::LiczCalk::Wew)
            qDebug() << "Sposób liczenia całki: wewnętrzny";
        else
            qDebug() << "Sposób liczenia całki: Zewnętrzny";

        qDebug() << "=====GEN=====";
        qDebug() << "Typ funkcji: ";
        if (m_gen_type == FunctionGenerator::FunctionType::SIN)
            qDebug() << "Sinusoidalny";
        else
            qDebug() << "Kwadratowy";
        qDebug() << "Amplituda: ";
        qDebug() << m_gen_amplitude;
        qDebug() << "Częstotliwość: ";
        qDebug() << m_gen_frequency;
        qDebug() << "Sampel(ms): ";
        qDebug() << m_gen_sample_ms;
        qDebug() << "Offset: ";
        qDebug() << m_gen_offset;
        qDebug() << "wypelnienie(tylko kwadratowy): ";
        qDebug() << m_gen_fill;
    }

signals:
    void SimulationUpdated();
private slots:
    void onTimerTimeout();
private:
    ServicesManager();

    QTimer *m_timer;
    double m_manual_setpoint = 0.0;

    // arx
    std::vector<double> m_arx_a = {0.0};
    std::vector<double> m_arx_b = {0.0};
    Borders m_border_u = {-10.0, 10.0};
    Borders m_border_y = {-10.0, 10.0};
    int m_arx_delay = 1;
    double m_arx_noise = 0;

    // pid
    double m_pid_p = 1.0;
    double m_pid_ti = 0.0;
    double m_pid_td = 0.0;
    RegulatorPID::LiczCalk m_LiczCalk = RegulatorPID::LiczCalk::Wew;


    // generatory
    FunctionGenerator::FunctionType m_gen_type = FunctionGenerator::FunctionType::SIN;
    double m_gen_amplitude = 1.0;
    double m_gen_frequency = 1.0;   // T_RZ
    int m_gen_sample_ms = 100;      // T_T
    double m_gen_offset = 0.0;
    double m_gen_fill = 0.5;        // dla square

    // obsluga
    bool m_use_generator = false;

    // glowny uar
    std::unique_ptr<UAR> m_uar = nullptr;
};


#endif // SERVICESMANAGER_H
