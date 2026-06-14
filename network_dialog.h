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
    explicit NetworkDialog(bool isConnected = false, QWidget *parent = nullptr);
    bool isServer() const;
    QString getIpAddress() const;

    bool wantsToDisconnect() const;

private:
    QRadioButton *radioServer;
    QRadioButton *radioClient;
    QLineEdit *ipInput;
    QPushButton *btnConnect;
    QPushButton *btnDisconnect;

    QLabel *localIpLabel;

    bool m_wantsToDisconnect;

    void setupUi(bool isConnected);
    QString getLocalIp();
};

#endif // NETWORK_DIALOG_H
