#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "NetworkProtocol.h"

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    // Tryb Serwera (nasłuchiwanie)
    bool startServer(int port);
    void stopServer();

    // Tryb Klienta (łączenie do serwera)
    void connectToServer(const QString& ip, int port);
    void disconnect();

    // Uniwersalna metoda do wysyłania paczek
    //void sendPacket(NetProto::MsgType type, const QByteArray& payload);
    void sendPacket(NetProto::MsgType type, const QByteArray& payload, uint32_t seqNum = 0);

    bool isConnected() const;

private slots:
    void onReadyRead();
    void onNewConnection(); // Dla serwera, gdy ktoś się podłączy
    void onConnected();     // Dla klienta, gdy uda się połączyć
    void onDisconnected();  // Gdy połączenie zostanie zerwane

private:
    QTcpSocket *m_socket;
    QTcpServer *m_server;

signals:
    void peerConnected(const QString& peerIp);
    void peerDisconnected();
    void configReceived();

    void arxConfigReceived(const QByteArray& payload);
    void pidConfigReceived(const QByteArray& payload);
    void genConfigReceived(const QByteArray& payload);

    void tickRegulatorReceived(double u, double w, uint32_t seqNum);
    void tickObjectReceived(double y, uint32_t seqNum);

    void intervalConfigReceived(int ms);
};

#endif // NETWORKMANAGER_H
