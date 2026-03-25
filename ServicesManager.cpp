#include "ServicesManager.h"


ServicesManager::ServicesManager() {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ServicesManager::onTimerTimeout);
    resetSimulation();
}

ServicesManager &ServicesManager::getInstance() {
    static ServicesManager instance;
    return instance;
}



void ServicesManager::startSimulation() {
    if (!m_timer->isActive()) {
        // Upewniamy się, że timer ma poprawny interwał
        if (m_timer->interval() <= 0) m_timer->setInterval(m_gen_sample_ms);
        m_timer->start();
    }
}

void ServicesManager::stopSimulation() {
    if (m_timer->isActive()) {
        m_timer->disconnect();
    }
}

void ServicesManager::setSimulationInterval(int ms) {
    m_gen_sample_ms = ms; // Aktualizujemy zmienną modelu
    m_timer->setInterval(ms); // Aktualizujemy timer
}

bool ServicesManager::isSimulationRunning() const {
    return m_timer->isActive();
}

void ServicesManager::setManualSetpoint(double val) {
    m_manual_setpoint = val;
}

void ServicesManager::onTimerTimeout() {
    runNextStep();
}

// Zmodyfikowana metoda runNextStep - nie przyjmuje argumentu, bierze z pola klasy
void ServicesManager::runNextStep() {
    if (!m_uar) return;

    if (m_use_generator) {
        m_uar->simulateWithGenerator();
    } else {
        // Używamy zapamiętanej wartości zadanej
        m_uar->simulate(m_manual_setpoint);
    }

    // Ważne: powiadamiamy GUI, że mamy nowe dane!
    emit SimulationUpdated();
}

void ServicesManager::setArxParams(const std::vector<double> &a, const std::vector<double> &b, int delay, double noise) {
    m_arx_a = a;
    m_arx_b = b;
    m_arx_delay = delay;
    m_arx_noise = noise;
}

void ServicesManager::setArxBorders(const Borders border_u, const Borders border_y) {
    m_border_u = border_u;
    m_border_y = border_y;
}

void ServicesManager::setPidParams(double p, double ti, double td, RegulatorPID::LiczCalk liczcalk) {
    m_pid_p = p;
    m_pid_ti = ti;
    m_pid_td = td;
    m_LiczCalk = liczcalk;
}

void ServicesManager::setGeneratorParams(FunctionGenerator::FunctionType type, double amp, double period_s, int sample_ms, double offset, double fill) {
    m_gen_amplitude = amp;
    m_gen_frequency = period_s; // T_RZ
    m_gen_sample_ms = sample_ms; // T_T
    m_gen_offset = offset;
    m_gen_fill = fill;
    m_gen_type = type;
}

void ServicesManager::setUseGenerator(bool use) {
    m_use_generator = use;
}

void ServicesManager::resetPidIntegral() {
    if (m_uar) {
        m_uar->getRegulatorPID().resetIntegral();
    }
}

void ServicesManager::applyParams() {
    if (!m_uar) {
        resetSimulation();
        return;
    }

    // ARX
    ARX &arx = m_uar->getARX();
    arx.setA(m_arx_a);
    arx.setB(m_arx_b);
    arx.setK(m_arx_delay);
    arx.setZ(m_arx_noise);
    arx.set_borders_u(m_border_u);
    arx.set_borders_y(m_border_y);

    // PID
    RegulatorPID &pid = m_uar->getRegulatorPID();
    pid.setK(m_pid_p);
    pid.setTi(m_pid_ti);
    pid.setTd(m_pid_td);
    pid.setLiczCalk(m_LiczCalk);

    // generator
    if (m_use_generator) {
        FunctionGenerator &gen = m_uar->getFunctionGenerator();
        gen.set_function_type(m_gen_type);
        gen.set_amplitude(m_gen_amplitude);
        gen.set_real_period_and_interval(m_gen_frequency, m_gen_sample_ms);
        gen.set_offset(m_gen_offset);
        gen.set_square_filling(m_gen_fill);
        m_timer->setInterval(m_gen_sample_ms);
    }
}


void ServicesManager::resetSimulation() {
    ARX arx_model(m_arx_a, m_arx_b, m_arx_delay);
    RegulatorPID regulator_pid(m_pid_p, m_pid_ti, m_pid_td);
    FunctionGenerator func_gen(m_gen_amplitude, m_gen_frequency, m_gen_sample_ms, m_gen_offset, m_gen_fill);
    func_gen.set_function_type(m_gen_type);

    m_uar = std::make_unique<UAR>(arx_model, regulator_pid, func_gen);
}

void ServicesManager::hardResetSimulation() {
    ARX arx_model({0.0}, {0.0}, 1);
    RegulatorPID regulator_pid(0,0,0);
    FunctionGenerator func_gen(0,0,0,0,0);
    func_gen.set_function_type(FunctionGenerator::FunctionType::SIN);

    m_uar = std::make_unique<UAR>(arx_model, regulator_pid, func_gen);
}

const std::deque<SimulationData> &ServicesManager::getSimulationData() const {
    return m_uar->getOutputHistory();
}

QJsonArray ServicesManager::vecToJson(const std::vector<double>& v)
{
    QJsonArray arr;
    for (double d : v) {
        arr.append(d);
    }
    return arr;
}

std::vector<double> ServicesManager::jsonToVec(const QJsonArray& arr)
{
    std::vector<double> v;
    for (const auto &val : arr) {
        v.push_back(val.toDouble());
    }
    return v;
}

bool ServicesManager::saveState(const QString& filePath)
{
    QJsonObject root;

    // ARX
    QJsonObject arxObj;
    arxObj["a"] = vecToJson(m_arx_a);
    arxObj["b"] = vecToJson(m_arx_b);
    arxObj["delay"] = m_arx_delay;
    arxObj["noise"] = m_arx_noise;

    QJsonObject bu, by;
    bu["bottom"] = m_border_u.bottom; bu["top"] = m_border_u.top;
    by["bottom"] = m_border_y.bottom; by["top"] = m_border_y.top;
    arxObj["border_u"] = bu;
    arxObj["border_y"] = by;

    root["arx"] = arxObj;

    // Pakowanie PID
    QJsonObject pidObj;
    pidObj["p"] = m_pid_p;
    pidObj["ti"] = m_pid_ti;
    pidObj["td"] = m_pid_td;
    pidObj["method"] = static_cast<int>(m_LiczCalk);
    root["pid"] = pidObj;

    // Pakowanie Generatora
    QJsonObject genObj;
    genObj["use"] = m_use_generator;
    genObj["type"] = static_cast<int>(m_gen_type);
    genObj["amp"] = m_gen_amplitude;
    genObj["freq"] = m_gen_frequency;
    genObj["sample"] = m_gen_sample_ms;
    genObj["offset"] = m_gen_offset;
    genObj["fill"] = m_gen_fill;
    root["gen"] = genObj;

    return SaveLoadManager::writeJsonToFile(filePath, root);
}

bool ServicesManager::loadState(const QString& filePath)
{
    QJsonObject root = SaveLoadManager::readJsonFromFile(filePath);

    if (root.isEmpty()) return false;

    // Rozpakowywanie danych do zmiennych prywatnych
    if (root.contains("arx")) {
        QJsonObject o = root["arx"].toObject();
        m_arx_a = jsonToVec(o["a"].toArray());
        m_arx_b = jsonToVec(o["b"].toArray());
        m_arx_delay = o["delay"].toInt();
        m_arx_noise = o["noise"].toDouble();

        QJsonObject bu = o["border_u"].toObject();
        m_border_u.bottom = bu["bottom"].toDouble(); m_border_u.top = bu["top"].toDouble();

        QJsonObject by = o["border_y"].toObject();
        m_border_y.bottom = by["bottom"].toDouble(); m_border_y.top = by["top"].toDouble();
    }

    if (root.contains("pid")) {
        QJsonObject o = root["pid"].toObject();
        m_pid_p = o["p"].toDouble();
        m_pid_ti = o["ti"].toDouble();
        m_pid_td = o["td"].toDouble();
        m_LiczCalk = static_cast<RegulatorPID::LiczCalk>(o["method"].toInt());
    }

    if (root.contains("gen")) {
        QJsonObject o = root["gen"].toObject();
        m_use_generator = o["use"].toBool();
        m_gen_type = static_cast<FunctionGenerator::FunctionType>(o["type"].toInt());
        m_gen_amplitude = o["amp"].toDouble();
        m_gen_frequency = o["freq"].toDouble();
        m_gen_sample_ms = o["sample"].toInt();
        m_gen_offset = o["offset"].toDouble();
        m_gen_fill = o["fill"].toDouble();
    }

    // Aplikujemy wczytane parametry do logiki symulacji
    applyParams();
    return true;
}
