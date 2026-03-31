#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include "NetworkProtocol.h" // Ten plik, który zdefiniowaliśmy wcześniej

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    // Metody do nawiązywania połączenia (rozbudujesz je później)
    void connectToServer(const QString& ip, int port);
    void disconnect();

    // Metoda do wysyłania danych (przykład z poprzedniej wiadomości)
    void sendConfigARX();
    void sendConfigPID();

private slots:
    // TUTAJ JEST MIEJSCE NA TWOJĄ FUNKCJĘ
    void onReadyRead();

private:
    QTcpSocket *m_socket;
};

#endif // NETWORKMANAGER_H
