#ifndef NETWORK_DIALOG_H
#define NETWORK_DIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>

class NetworkDialog : public QDialog {
    Q_OBJECT
public:
    // --- ZMODYFIKOWANE: Parametr isConnected ---
    explicit NetworkDialog(bool isConnected = false, QWidget *parent = nullptr);
    bool isServer() const;
    QString getIpAddress() const;

    // --- NOWE: Metoda sprawdzająca czy kliknięto rozłącz ---
    bool wantsToDisconnect() const;

private:
    QRadioButton *radioServer;
    QRadioButton *radioClient;
    QLineEdit *ipInput;
    QPushButton *btnConnect;
    QPushButton *btnDisconnect; // <--- NOWE

    QLabel *localIpLabel;

    bool m_wantsToDisconnect; // <--- NOWE

    void setupUi(bool isConnected); // Zmodyfikowana sygnatura
    QString getLocalIp();
};

#endif // NETWORK_DIALOG_H
