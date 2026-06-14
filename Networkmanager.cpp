#include "NetworkManager.h"
#include "ServicesManager.h" // Żeby mieć dostęp do UAR
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
    m_socket = new QTcpSocket(this);
    m_server = new QTcpServer(this);

    // Sygnały dla klienta
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);

    // Sygnały dla serwera
    connect(m_server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);
}

// --- TRYB SERWERA ---
bool NetworkManager::startServer(int port) {
    if (m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "Serwer nasłuchuje na porcie:" << port;
        return true;
    }
    qDebug() << "Błąd stawiania serwera:" << m_server->errorString();
    return false;
}

void NetworkManager::stopServer() {
    m_server->close();
    disconnect();
    qDebug() << "Serwer zatrzymany.";
}

void NetworkManager::onNewConnection() {
    // Serwer akceptuje nowe połączenie
    QTcpSocket* clientSocket = m_server->nextPendingConnection();

    // Zastępujemy nasz domyślny socket tym nowym, połączonym
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Ktoś już jest połączony, odrzucam nowe połączenie.";
        clientSocket->disconnectFromHost();
        return;
    }

    m_socket = clientSocket;
    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1); //wyłączanie nagle'a

    // Przepinamy sygnały do nowego gniazda
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);

    qDebug() << "Nowy klient podłączony z IP:" << m_socket->peerAddress().toString();
    emit peerConnected(m_socket->peerAddress().toString());
}

// --- TRYB KLIENTA ---
void NetworkManager::connectToServer(const QString& ip, int port) {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        disconnect();
    }
    qDebug() << "Próba połączenia z:" << ip << ":" << port;
    m_socket->connectToHost(ip, port);
}

void NetworkManager::disconnect() {
    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
    }
}

void NetworkManager::onConnected() {
    qDebug() << "Połączono z serwerem pomyślnie!";

    m_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1); // Wyłącza Nagle'a
    emit peerConnected(m_socket->peerAddress().toString());

}

void NetworkManager::onDisconnected() {
    qDebug() << "Rozłączono.";
    emit peerDisconnected();
}

bool NetworkManager::isConnected() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

// --- WYSYŁANIE ---
void NetworkManager::sendPacket(NetProto::MsgType type, const QByteArray& payload, uint32_t seqNum) {
    if (!isConnected()) {
        qDebug() << "Brak połączenia! Nie można wysłać danych.";
        return;
    }

    NetProto::PacketHeader header;
    header.totalSize = sizeof(NetProto::PacketHeader) + payload.size();
    header.type = type;
    header.seqNum = seqNum;

    QByteArray packet;
    packet.reserve(header.totalSize);

    packet.append(reinterpret_cast<const char*>(&header), sizeof(NetProto::PacketHeader));
    packet.append(payload);

    m_socket->write(packet);
    m_socket->flush();
}

void NetworkManager::onReadyRead() {
    // Dopóki mamy w buforze wystarczająco dużo danych, żeby chociaż przeczytać nagłówek
    while(m_socket->bytesAvailable() >= static_cast<qint64>(sizeof(NetProto::PacketHeader))) {

        NetProto::PacketHeader header;
        // Podglądamy nagłówek BEZ ściągania go z bufora soketu
        m_socket->peek(reinterpret_cast<char*>(&header), sizeof(NetProto::PacketHeader));

        // Zabezpieczenie czy cały pakiet dotarł (na wypadek, gdyby przyszła tylko część danych)
        if(m_socket->bytesAvailable() < static_cast<qint64>(header.totalSize)) {
            return; // Czekamy na resztę pakietu
        }

        // Cała ramka z Socketu
        QByteArray fullPacket = m_socket->read(header.totalSize);

        QByteArray payload = fullPacket.mid(sizeof(NetProto::PacketHeader));

        // Deserializacja w zależności od tego, co przyszło

        switch(header.type) {
        case NetProto::MsgType::CONFIG_ARX:
            emit arxConfigReceived(payload);
            break;
        case NetProto::MsgType::CONFIG_PID:
            emit pidConfigReceived(payload);
            break;
        case NetProto::MsgType::CONFIG_GEN:
            emit genConfigReceived(payload);
            break;
        case NetProto::MsgType::TICK_REGULATOR: {
            NetProto::PayloadTickRegulator p;
            memcpy(&p, payload.constData(), sizeof(p));
            emit tickRegulatorReceived(p.u, p.w, header.seqNum);
            break;
        }
        case NetProto::MsgType::TICK_OBJECT: {
            NetProto::PayloadTickObject p;
            memcpy(&p, payload.constData(), sizeof(p));
            emit tickObjectReceived(p.y, header.seqNum);
            break;
        }
        case NetProto::MsgType::CONFIG_INTERVAL: {
            int interval;
            memcpy(&interval, payload.constData(), sizeof(int));
            emit intervalConfigReceived(interval);
            break;
        }
        case NetProto::MsgType::SIM_CTRL: {
            NetProto::PayloadSimCtrl p;
            memcpy(&p, payload.constData(), sizeof(p)); // Kopiujemy bajty z payloadu do struktury

            emit simCommandReceived(p.command);
            break;
        }
        default:
            qDebug() << "Odebrano wiadomosc nieobslugiwanego typu.";
            break;
        }

    }
}

