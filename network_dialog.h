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
    explicit NetworkDialog(QWidget *parent = nullptr);
    bool isServer() const;
    QString getIpAddress() const;

private:
    QRadioButton *radioServer;
    QRadioButton *radioClient;
    QLineEdit *ipInput;
    QPushButton *btnConnect;
    QLabel *localIpLabel;

    void setupUi();
    QString getLocalIp();
};

#endif // NETWORK_DIALOG_H
