#include "network_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkInterface>
#include <QHostAddress>

NetworkDialog::NetworkDialog(QWidget *parent) : QDialog(parent) {
    setupUi();

    // Zablokuj wpisywanie IP jeśli jesteśmy serwerem (bo serwer sam nasłuchuje)
    connect(radioServer, &QRadioButton::toggled, ipInput, [this](bool checked){
        ipInput->setDisabled(checked);
    });

    connect(btnConnect, &QPushButton::clicked, this, &QDialog::accept);
}

void NetworkDialog::setupUi() {
    setWindowTitle("Konfiguracja połączenia sieciowego");
    setMinimumWidth(300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Tryby
    radioServer = new QRadioButton("Serwer (Instancja Obiektu - nasłuchuje)", this);
    radioClient = new QRadioButton("Klient (Instancja Regulatora - łączy się)", this);
    radioClient->setChecked(true); // Domyślnie klient

    mainLayout->addWidget(radioServer);
    mainLayout->addWidget(radioClient);

    // IP Do połączenia
    QHBoxLayout *ipLayout = new QHBoxLayout();
    ipLayout->addWidget(new QLabel("IP do połączenia:", this));
    ipInput = new QLineEdit("127.0.0.1", this);
    ipLayout->addWidget(ipInput);
    mainLayout->addLayout(ipLayout);

    // Wyświetlanie lokalnego IP maszyny (dla ułatwienia życia z PDF)
    localIpLabel = new QLabel("Lokalne IP: <b>" + getLocalIp() + "</b>", this);
    mainLayout->addWidget(localIpLabel);

    // Przycisk
    btnConnect = new QPushButton("Zatwierdź", this);
    mainLayout->addWidget(btnConnect);
}

QString NetworkDialog::getLocalIp() {
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            return address.toString(); // Zwraca pierwszy sensowny adres IPv4
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
