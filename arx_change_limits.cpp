#include "arx_change_limits.h"
#include "ui_arx_change_limits.h"

arx_change_limits::arx_change_limits(QWidget *parent, double U_MINIMAL, double U_MAXIMAL, double Y_MINIMAL, double Y_MAXIMAL)
    : QDialog(parent)
    , ui(new Ui::arx_change_limits)
{
    ui->setupUi(this);

    this->setWindowTitle("Zmiana limitów modelu ARX");
    this->setAttribute(Qt::WA_DeleteOnClose);

    if (std::abs(U_MAXIMAL) > (NO_LIMIT_VAL - 1000.0)) {
        // Jeśli limit jest wyłączony (ogromna liczba)
        ui->checkBox_turnoff_U->setChecked(true);
        ui->doubleSpinBox_U_MIN->setValue(0.0);
        ui->doubleSpinBox_U_MAX->setValue(0.0);
    } else {
        // Normalne limity
        ui->checkBox_turnoff_U->setChecked(false);
        ui->doubleSpinBox_U_MIN->setValue(U_MINIMAL);
        ui->doubleSpinBox_U_MAX->setValue(U_MAXIMAL);
    }

    if (std::abs(Y_MAXIMAL) > (NO_LIMIT_VAL - 1000.0)) {
        // Jeśli limit jest wyłączony (ogromna liczba)
        ui->checkBox_turnoff_Y->setChecked(true);
        ui->doubleSpinBox_Y_MIN->setValue(0.0);
        ui->doubleSpinBox_Y_MAX->setValue(0.0);
    } else {
        // Normalne limity
        ui->checkBox_turnoff_Y->setChecked(false);
        ui->doubleSpinBox_Y_MIN->setValue(Y_MINIMAL);
        ui->doubleSpinBox_Y_MAX->setValue(Y_MAXIMAL);
    }
}

arx_change_limits::~arx_change_limits()
{
    delete ui;
}

void arx_change_limits::on_pushButton_submit_clicked()
{

    double U_min, U_max, Y_min, Y_max;

    if (ui->checkBox_turnoff_U->isChecked()) {
        // Limit wyłączony -> ustawiamy zakres "nieskończony"
        U_min = -NO_LIMIT_VAL;
        U_max = NO_LIMIT_VAL;
    } else {
        // Limit włączony -> bierzemy z interfejsu
        U_min = ui->doubleSpinBox_U_MIN->value();
        U_max = ui->doubleSpinBox_U_MAX->value();

        if (U_min >= U_max) {
            QMessageBox::warning(this, "Błąd danych", "Dla U: Wartość MIN musi być mniejsza od MAX!");
            return;
        }
    }

    if (ui->checkBox_turnoff_Y->isChecked()) {
        // Limit wyłączony
        Y_min = -NO_LIMIT_VAL;
        Y_max = NO_LIMIT_VAL;
    } else {
        // Limit włączony
        Y_min = ui->doubleSpinBox_Y_MIN->value();
        Y_max = ui->doubleSpinBox_Y_MAX->value();

        if (Y_min >= Y_max) {
            QMessageBox::warning(this, "Błąd danych", "Dla Y: Wartość MIN musi być mniejsza od MAX!");
            return;
        }
    }

    emit updateARXlimits(U_min, U_max, Y_min, Y_max);

    accept();
}

void arx_change_limits::on_pushButton_cancel_clicked()
{
    reject();
}

void arx_change_limits::on_checkBox_turnoff_U_stateChanged(int arg1)
{
    bool disableInputs = (arg1 != 0);

    ui->doubleSpinBox_U_MIN->setDisabled(disableInputs);
    ui->doubleSpinBox_U_MAX->setDisabled(disableInputs);
}


void arx_change_limits::on_checkBox_turnoff_Y_stateChanged(int arg1)
{
    bool disableInputs = (arg1 != 0);

    ui->doubleSpinBox_Y_MIN->setDisabled(disableInputs);
    ui->doubleSpinBox_Y_MAX->setDisabled(disableInputs);
}

