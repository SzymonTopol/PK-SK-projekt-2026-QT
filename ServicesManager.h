#ifndef SERVICESMANAGER_H
#define SERVICESMANAGER_H

#include "UAR.h"
#include "saveloadmanager.h"

#include <QObject>
#include <QTimer>
#include <vector>
#include <QDebug>
#include <deque>

#include "Networkmanager.h"

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

    //Pod przesyłanie sieciowe
    UAR* getUar() { return m_uar.get(); }
    NetworkManager* getNetworkManager() const { return m_networkManager.get(); }

    bool isServerMode() const { return m_is_server_mode; }

    void testSerialization();

    void setupAsServer();
    void connectAndSendConfigAsClient(const QString& ip = "127.0.0.1");

signals:
    void SimulationUpdated();
    void peerConnectionChanged(bool connected, const QString& ip);
    void networkConfigReceived();
    void syncStatusChanged(bool inSync); // Poinformuje GUI czy symulacja się wyrabia
    void simCommandReceived(NetProto::SimCommand cmd);
private slots:
    void onTimerTimeout();
private:
    ServicesManager();

    QTimer *m_timer;
    double m_manual_setpoint = 0.0;

    // arx
    std::vector<double> m_arx_a = {-0.4, -0.6, 0.0};
    std::vector<double> m_arx_b = {0.6, 0.4, 0.0};
    Borders m_border_u = {-1.0, 1.0};
    Borders m_border_y = {-1.0, 1.0};
    int m_arx_delay = 1;
    double m_arx_noise = 0.0;

    // pid
    double m_pid_p = 0.5;
    double m_pid_ti = 5.0;
    double m_pid_td = 0.0;
    RegulatorPID::LiczCalk m_LiczCalk = RegulatorPID::LiczCalk::Zew;

    // generatory
    FunctionGenerator::FunctionType m_gen_type = FunctionGenerator::FunctionType::SQUARE;
    double m_gen_amplitude = 1.0;
    double m_gen_frequency = 10.0;   // T_RZ
    int m_gen_sample_ms = 50;        // T_T
    double m_gen_offset = 0.0;
    double m_gen_fill = 0.5;         // dla square

    // obsluga
    bool m_use_generator = true;

    // glowny uar
    std::unique_ptr<UAR> m_uar = nullptr;

    std::unique_ptr<NetworkManager> m_networkManager;

    void broadcastConfiguration();

    bool m_is_server_mode = false;
     bool m_logical_is_running = false;
    bool m_received_y_for_current_step = true; // Flaga wyrabiania się
    double m_last_known_y = 0.0;
    ClientTickData m_last_client_tick;
    uint32_t m_current_step = 0;

    // Licznik zgubionych ramek z rzędu dla Klienta
    int m_consecutive_drops = 0;
    // Próg, po którym zrywamy połączenie
    const int MAX_DROPS_BEFORE_DISCONNECT = 20;

    // Ostatni przetworzony numer sekwencyjny przez Serwer
    uint32_t m_last_server_seq = 0;



    std::deque<bool> m_drop_history;  // true = zgubiona ramka, false = dotarła
    int m_drops_in_history = 0;       // aktualna suma zgubionych ramek w oknie
};


#endif // SERVICESMANAGER_H
