#include "NetworkManager.h"
#include "ServicesManager.h" // Żeby mieć dostęp do UAR
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    m_socket = new QTcpSocket(this);

    // KRYTYCZNE: Łączymy sygnał z gniazda z Twoim kodem!
    // Kiedy przyjdą dane, automatycznie odpali się onReadyRead()
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
}

void NetworkManager::onReadyRead() {
    // Dopóki mamy w buforze wystarczająco dużo danych, żeby chociaż przeczytać nagłówek
    while(m_socket->bytesAvailable() >= static_cast<qint64>(sizeof(NetProto::PacketHeader))) {

        NetProto::PacketHeader header;
        // Podglądamy nagłówek BEZ ściągania go z bufora soketu
        m_socket->peek(reinterpret_cast<char*>(&header), sizeof(NetProto::PacketHeader));

        // Zabezpieczenie: Czy cały pakiet już dojechał? QTcpSocket czasem tnie pakiety na raty!
        if(m_socket->bytesAvailable() < static_cast<qint64>(header.totalSize)) {
            return; // Czekamy na resztę pakietu
        }

        // Ściągamy całą ramkę z soketu
        QByteArray fullPacket = m_socket->read(header.totalSize);

        // Wycinamy sam payload (wszystko po nagłówku)
        QByteArray payload = fullPacket.mid(sizeof(NetProto::PacketHeader));

        // Deserializacja w zależności od tego, co przyszło
        switch(header.type) {
        case NetProto::MsgType::CONFIG_ARX: {
            // Odwołujemy się do Singletona ServicesManager, żeby dobrać się do instancji UAR
            if(ServicesManager::getInstance().getUar() != nullptr) {
                ServicesManager::getInstance().getUar()->getARX().deserializeConfig(payload);
                qDebug() << "Odebrano nową konfigurację ARX!";
            }
            break;
        }
        case NetProto::MsgType::CONFIG_PID: {
            if(ServicesManager::getInstance().getUar() != nullptr) {
                ServicesManager::getInstance().getUar()->getRegulatorPID().deserializeConfig(payload);
                qDebug() << "Odebrano nową konfigurację PID!";
            }
            break;
        }
        default:
            qDebug() << "Odebrano wiadomosc nieobslugiwanego typu.";
                break;
        }
    }
}
