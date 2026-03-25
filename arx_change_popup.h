#ifndef ARX_CHANGE_POPUP_H
#define ARX_CHANGE_POPUP_H

#include "qscrollarea.h"
#include <QDialog>
#include <vector>
#include <QDoubleSpinBox>
#include <QVBoxLayout>

namespace Ui {
class ARX_change_popup;
}

class ARX_change_popup : public QDialog
{
    Q_OBJECT

public:
    explicit ARX_change_popup(const std::vector<double> &currentA, const std::vector<double> &currentB, const int current_delay, const double currnet_noise, QWidget *parent = nullptr);
    ~ARX_change_popup();

signals:
    void updateArxData(const std::vector<double> &a, const std::vector<double> &b, int delay, double noise);

private slots:
    void on_btn_cancel_clicked();
    void on_btn_save_clicked();
    // Slot reagujący na zmianę wspólnego licznika
    void updateListsCount(int count);

private:
    Ui::ARX_change_popup *ui;

    std::vector<QDoubleSpinBox*> inputsA;
    std::vector<QDoubleSpinBox*> inputsB;

    void updateInputList(int count, QScrollArea* scrollArea, std::vector<QDoubleSpinBox*> &list);
    void clearScrollArea(QScrollArea* scrollArea);
};

#endif // ARX_CHANGE_POPUP_H
