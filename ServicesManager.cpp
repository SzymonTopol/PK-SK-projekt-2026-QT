#include "ServicesManager.h"


ServicesManager::ServicesManager() {
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &ServicesManager::onTimerTimeout);
    resetSimulation();
    m_networkManager = std::make_unique<NetworkManager>(this);

    //sygnały sieciowe
    connect(m_networkManager.get(), &NetworkManager::peerConnected, this, [this](const QString& ip){
        emit peerConnectionChanged(true, ip);
    });
    // connect(m_networkManager.get(), &NetworkManager::peerDisconnected, this, [this](){
    //     emit peerConnectionChanged(false, "");
    // });

    connect(m_networkManager.get(), &NetworkManager::peerDisconnected, this, [this](){
        emit peerConnectionChanged(false, ""); // To już powiadomi GUI i wyrzuci QMessageBox

        // 1. Obie aplikacje stają się stacjonarne (wyłączamy tryb serwera)
        m_is_server_mode = false;

        // 2. Jeśli Klient zawiesił się w oczekiwaniu na pakiet (czerwona flaga), zdejmujemy blokadę
        m_received_y_for_current_step = true;

        // 3. Wymuszamy start zegara. Jeśli to był Serwer, jego timer stał, więc teraz ruszy
        // i będzie kontynuował na lokalnych parametrach[cite: 36].
        startSimulation();
    });
    // connect(m_networkManager.get(), &NetworkManager::configReceived, this, [this](){
    //     emit networkConfigReceived();
    // });
    connect(m_networkManager.get(), &NetworkManager::arxConfigReceived, this, [this](const QByteArray& payload){
        if(m_uar) {
            m_uar->getARX().deserializeConfig(payload);

            m_arx_a = m_uar->getARX().getA();
            m_arx_b = m_uar->getARX().getB();
            m_arx_delay = m_uar->getARX().getK();
            m_arx_noise = m_uar->getARX().getZ();

            m_border_u = m_uar->getARX().getBordersU();
            m_border_y = m_uar->getARX().getBordersY();

            emit networkConfigReceived();
        }
    });

    connect(m_networkManager.get(), &NetworkManager::pidConfigReceived, this, [this](const QByteArray& payload){
        if(m_uar) {
            m_uar->getRegulatorPID().deserializeConfig(payload);

            m_pid_p = m_uar->getRegulatorPID().getK();
            m_pid_ti = m_uar->getRegulatorPID().getTi();
            m_pid_td = m_uar->getRegulatorPID().getTd();
            m_LiczCalk = m_uar->getRegulatorPID().getLiczCalk();

            emit networkConfigReceived();
        }
    });

    connect(m_networkManager.get(), &NetworkManager::genConfigReceived, this, [this](const QByteArray& payload){
        if(m_uar) {
            m_uar->getFunctionGenerator().deserializeConfig(payload);

            m_gen_amplitude = m_uar->getFunctionGenerator().getAmplitude();
            m_gen_offset = m_uar->getFunctionGenerator().getOffset();
            m_gen_fill = m_uar->getFunctionGenerator().getSquareFilling();
            m_gen_type = m_uar->getFunctionGenerator().getType();

            m_gen_frequency = (m_uar->getFunctionGenerator().getT() * m_gen_sample_ms) / 1000.0;

            emit networkConfigReceived();
        }
    });

    // ODBIÓR NA SERWERZE
    /*connect(m_networkManager.get(), &NetworkManager::tickRegulatorReceived, this, [this](double u, double w, uint32_t seqNum){
        if (m_uar && m_is_server_mode) {
            // 1. Liczymy wyjście z obiektu
            double y = m_uar->run_server_calc(u);

            // 2. Wrzucamy do historii i odświeżamy wykresy
            m_uar->commit_server_step(w, u, y, seqNum);
            emit SimulationUpdated();

            // 3. Odsyłamy szybciutko 'y' z powrotem do Klienta
            NetProto::PayloadTickObject payload;
            payload.y = y;
            QByteArray data(reinterpret_cast<const char*>(&payload), sizeof(payload));
            m_networkManager->sendPacket(NetProto::MsgType::TICK_OBJECT, data, seqNum);
        }
    });*/

    connect(m_networkManager.get(), &NetworkManager::tickRegulatorReceived, this, [this](double u, double w, uint32_t seqNum){
        if (m_uar && m_is_server_mode) {
            // 1. Liczymy wyjście z obiektu
            double y = m_uar->run_server_calc(u);

            // 2. NAJPIERW odsyłamy 'y' z powrotem do Klienta (odblokowujemy go!)
            NetProto::PayloadTickObject payload;
            payload.y = y;
            QByteArray data(reinterpret_cast<const char*>(&payload), sizeof(payload));
            m_networkManager->sendPacket(NetProto::MsgType::TICK_OBJECT, data, seqNum);

            // 3. DOPIERO POTEM wrzucamy do historii i odświeżamy wykresy u siebie
            m_uar->commit_server_step(w, u, y, seqNum);
            emit SimulationUpdated();
        }
    });

    // ODBIÓR NA KLIENCIE
    connect(m_networkManager.get(), &NetworkManager::tickObjectReceived, this, [this](double y, uint32_t seqNum){
        if (m_uar && !m_is_server_mode) {
            // Czy odpowiedź dotyczy obecnego kroku?
            if (seqNum == m_current_step - 1) {
                m_received_y_for_current_step = true; // ZDĄŻYŁ!
                m_last_known_y = y;
                emit syncStatusChanged(true); // Zielone światło w GUI

                // Zapisujemy wyliczony krok w historii i rysujemy go na wykresie
                m_uar->commit_client_step(m_last_client_tick, y, seqNum);
                emit SimulationUpdated();
            }
        }
    });

    connect(m_networkManager.get(), &NetworkManager::intervalConfigReceived, this, [this](int ms){
        m_gen_sample_ms = ms;
        m_timer->setInterval(ms);
        // Wywołanie tego sygnału odpali Twoje UpdateUIAfterLoad() w MainWindow
        // i pięknie zaktualizuje położenie suwaków u Serwera
        emit networkConfigReceived();
    });
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
        m_timer->stop();
    }
}

void ServicesManager::setSimulationInterval(int ms) {
    m_gen_sample_ms = ms;
    m_timer->setInterval(ms);

    // Jeśli jesteśmy KLIENTEM i jesteśmy połączeni, powiadamiamy serwer o nowym czasie
    if (m_networkManager && m_networkManager->isConnected() && !m_is_server_mode) {
        QByteArray data(reinterpret_cast<const char*>(&ms), sizeof(int));
        m_networkManager->sendPacket(NetProto::MsgType::CONFIG_INTERVAL, data);
    }
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

    // Wartość zadana (Generator lub stała)
    double setpoint = m_use_generator ? m_uar->getFunctionGenerator().get_value(m_current_step) : m_manual_setpoint;

    // 1. TRYB STACJONARNY (jeśli brak neta)
    if (!m_networkManager->isConnected()) {
        m_uar->simulate(setpoint);
        m_current_step++;
        emit SimulationUpdated();
        return;
    }

    // 2. TRYB SIECIOWY - SERWER (Ignoruje uderzenia lokalnego timera!)
    if (m_is_server_mode) {
        return;
    }

    // 3. TRYB SIECIOWY - KLIENT (Główny narzucający tempo)
    // Sprawdzamy czy poprzedni pakiet zdążył wrócić
    if (!m_received_y_for_current_step && m_current_step > 0) {
        emit syncStatusChanged(false); // ZGUBIONY PAKIET! (Czerwone światło)

        // Program MUSI pracować dalej (wymóg PDF), więc ratujemy się starym 'y'
        m_uar->commit_client_step(m_last_client_tick, m_last_known_y, m_current_step - 1);
        emit SimulationUpdated();
    }

    // Wyliczamy pierwszą połówkę nowego kroku
    m_last_client_tick = m_uar->run_client_calc(setpoint, m_last_known_y);

    // Pakujemy sygnał sterujący (u) i wartość zadaną (w) dla Serwera
    NetProto::PayloadTickRegulator payload;
    payload.u = m_last_client_tick.u;
    payload.w = setpoint;

    QByteArray data(reinterpret_cast<const char*>(&payload), sizeof(payload));
    m_networkManager->sendPacket(NetProto::MsgType::TICK_REGULATOR, data, m_current_step);

    m_received_y_for_current_step = false; // Zakładamy opóźnienie, aż nie przyjdzie paczka
    m_current_step++;
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

    broadcastConfiguration();
}


void ServicesManager::resetSimulation() {
    ARX arx_model(m_arx_a, m_arx_b, m_arx_delay);
    RegulatorPID regulator_pid(m_pid_p, m_pid_ti, m_pid_td);
    FunctionGenerator func_gen(m_gen_amplitude, m_gen_frequency, m_gen_sample_ms, m_gen_offset, m_gen_fill);
    func_gen.set_function_type(m_gen_type);

    m_uar = std::make_unique<UAR>(arx_model, regulator_pid, func_gen);
}

void ServicesManager::hardResetSimulation() {
    // Twardy reset ARX
    ARX arx_model({-0.4, -0.6, 0.0}, {0.6, 0.4, 0.0}, 1);
    arx_model.set_borders_u({-1.0, 1.0});
    arx_model.set_borders_y({-1.0, 1.0});

    // Twardy reset PID
    RegulatorPID regulator_pid(0.5, 5.0, 0.0);
    regulator_pid.setLiczCalk(RegulatorPID::LiczCalk::Zew);

    // Twardy reset Generatora
    FunctionGenerator func_gen(1.0, 10.0, 50, 0.0, 0.5);
    func_gen.set_function_type(FunctionGenerator::FunctionType::SQUARE);

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

void ServicesManager::testSerialization() {
    qDebug() << "=== START TESTU SERIALIZACJI ===";

    if (!m_uar) {
        qDebug() << "Błąd: UAR nie jest zainicjalizowany! Odpal najpierw symulację.";
        return;
    }

    // 1. Zapisujemy obecny stan ARX do QByteArray (symulujemy NADAWCĘ)
    QByteArray payload = m_uar->getARX().serializeConfig();

    // Budujemy nagłówek tak, jakbyśmy wysyłali to przez sieć
    NetProto::PacketHeader header;
    header.totalSize = sizeof(NetProto::PacketHeader) + payload.size();
    header.type = NetProto::MsgType::CONFIG_ARX;
    header.seqNum = 0;

    QByteArray packet;
    packet.append(reinterpret_cast<const char*>(&header), sizeof(NetProto::PacketHeader));
    packet.append(payload);

    qDebug() << "Paczka gotowa! Rozmiar całkowity:" << packet.size() << "bajtów.";

    // 2. Symulujemy przesył... Psujemy lokalnego ARXa, żeby udowodnić, że odzyskamy dane z paczki!
    std::vector<double> fakeA = {999.9, 999.9};
    m_uar->getARX().setA(fakeA);
    m_uar->getARX().setK(999);
    qDebug() << "Zepsuto ARXa. Zaraz spróbujemy go odzyskać z paczki binarnej...";

    // 3. Odbiór (symulujemy działanie onReadyRead z NetworkManager)
    NetProto::PacketHeader receivedHeader;
    memcpy(&receivedHeader, packet.data(), sizeof(NetProto::PacketHeader));

    if (receivedHeader.type == NetProto::MsgType::CONFIG_ARX) {
        // Wycinamy sam payload (wszystko po nagłówku)
        QByteArray receivedPayload = packet.mid(sizeof(NetProto::PacketHeader));

        // Przywracamy dane z paczki do naszego ARXa
        m_uar->getARX().deserializeConfig(receivedPayload);
        qDebug() << "Odzyskano dane ARX z paczki binarnej!";
    }

    qDebug() << "=== KONIEC TESTU ===";
}

// --- FUNKCJE SIECIOWE ---

void ServicesManager::setupAsServer() {
    m_is_server_mode = true;
    if(m_networkManager) m_networkManager->startServer(12345);
}

void ServicesManager::connectAndSendConfigAsClient(const QString& ip) {
    if(!m_networkManager) return;
    m_is_server_mode = false;
    m_networkManager->connectToServer(ip, 12345);

    QTimer::singleShot(500, this, [this]() {
        if(m_uar && m_networkManager->isConnected()) {
            // ARX
            QByteArray arxPayload = m_uar->getARX().serializeConfig();
            m_networkManager->sendPacket(NetProto::MsgType::CONFIG_ARX, arxPayload);
            qDebug() << "Wysłano konfigurację ARX!";

            // PID
            QByteArray pidPayload = m_uar->getRegulatorPID().serializeConfig();
            m_networkManager->sendPacket(NetProto::MsgType::CONFIG_PID, pidPayload);
            qDebug() << "Wysłano konfigurację PID!";

            // GENERATOR
            QByteArray genPayload = m_uar->getFunctionGenerator().serializeConfig();
            m_networkManager->sendPacket(NetProto::MsgType::CONFIG_GEN, genPayload);
            qDebug() << "Wysłano konfigurację Generatora!";

            // --- NOWE: WYSYŁANIE STARTOWEGO INTERWAŁU ---
            QByteArray intervalData(reinterpret_cast<const char*>(&m_gen_sample_ms), sizeof(int));
            m_networkManager->sendPacket(NetProto::MsgType::CONFIG_INTERVAL, intervalData);
            qDebug() << "Wysłano początkowy interwał symulacji (" << m_gen_sample_ms << "ms)!";

        } else {
            qDebug() << "Błąd: Nie udało się połączyć lub brak symulacji (UAR).";
        }
    });
}

void ServicesManager::broadcastConfiguration() {
    // Sprawdzamy czy symulacja żyje i czy jesteśmy połączeni z drugą aplikacją
    if (!m_uar || !m_networkManager || !m_networkManager->isConnected()) {
        return;
    }

    // Wysyłamy paczki z nową konfiguracją
    m_networkManager->sendPacket(NetProto::MsgType::CONFIG_ARX, m_uar->getARX().serializeConfig());
    m_networkManager->sendPacket(NetProto::MsgType::CONFIG_PID, m_uar->getRegulatorPID().serializeConfig());
    m_networkManager->sendPacket(NetProto::MsgType::CONFIG_GEN, m_uar->getFunctionGenerator().serializeConfig());

    qDebug() << "Rozgłoszono nową konfigurację przez sieć!";
}
