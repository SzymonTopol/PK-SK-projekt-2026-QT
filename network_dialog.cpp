#include "network_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkInterface>
#include <QHostAddress>

NetworkDialog::NetworkDialog(bool isConnected, QWidget *parent)
    : QDialog(parent), m_wantsToDisconnect(false)
{
    setupUi(isConnected);

    // Zablokuj wpisywanie IP jeśli jesteśmy serwerem
    connect(radioServer, &QRadioButton::toggled, ipInput, [this](bool checked){
        ipInput->setDisabled(checked);
    });

    connect(btnConnect, &QPushButton::clicked, this, &QDialog::accept);

    connect(btnDisconnect, &QPushButton::clicked, this, [this](){
        m_wantsToDisconnect = true;
        accept();
    });
}

void NetworkDialog::setupUi(bool isConnected) {
    setWindowTitle("Konfiguracja połączenia sieciowego");
    setMinimumWidth(300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    radioServer = new QRadioButton("Serwer (Instancja Obiektu - nasłuchuje)", this);
    radioClient = new QRadioButton("Klient (Instancja Regulatora - łączy się)", this);
    radioClient->setChecked(true);

    mainLayout->addWidget(radioServer);
    mainLayout->addWidget(radioClient);

    QHBoxLayout *ipLayout = new QHBoxLayout();
    ipLayout->addWidget(new QLabel("IP do połączenia:", this));
    ipInput = new QLineEdit("127.0.0.1", this);
    ipLayout->addWidget(ipInput);
    mainLayout->addLayout(ipLayout);

    localIpLabel = new QLabel("Lokalne IP: <b>" + getLocalIp() + "</b>", this);
    mainLayout->addWidget(localIpLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnConnect = new QPushButton("Zatwierdź / Połącz", this);
    btnDisconnect = new QPushButton("Rozłącz", this);

    btnConnect->setDisabled(isConnected);
    btnDisconnect->setEnabled(isConnected);

    btnLayout->addWidget(btnConnect);
    btnLayout->addWidget(btnDisconnect);
    mainLayout->addLayout(btnLayout);
}

QString NetworkDialog::getLocalIp() {
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            return address.toString();
        }
    }
    return "127.0.0.1";
}

bool NetworkDialog::isServer() const {
    return radioServer->isChecked();
}

QString NetworkDialog::getIpAddress() const {
    return ipInput->text();
}

bool NetworkDialog::wantsToDisconnect() const {
    return m_wantsToDisconnect;
}
