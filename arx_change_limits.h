#ifndef ARX_CHANGE_LIMITS_H
#define ARX_CHANGE_LIMITS_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class arx_change_limits;
}

class arx_change_limits : public QDialog
{
    Q_OBJECT

public:
    explicit arx_change_limits(QWidget *parent = nullptr, double U_MINIMAL = -10.0, double U_MAXIMAL = 10.0, double Y_MINIMAL = -10.0, double Y_MAXIMAL = 10.0);
    ~arx_change_limits();

signals:
    void updateARXlimits(const double U_min, const double U_max, const double Y_min, const double Y_max);

private slots:
    void on_pushButton_submit_clicked();
    void on_pushButton_cancel_clicked();

    void on_checkBox_turnoff_U_stateChanged(int arg1);
    void on_checkBox_turnoff_Y_stateChanged(int arg1);

private:
    Ui::arx_change_limits *ui;

    //nieskończoność, bo jak taką wartość ustawimy to w ARX warunek sprawdzający zakresy będzie prawdziwy
    const double NO_LIMIT_VAL = 999999999.0;
};

#endif // ARX_CHANGE_LIMITS_H
