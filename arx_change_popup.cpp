#include "arx_change_popup.h"
#include "ui_arx_change_popup.h"

ARX_change_popup::ARX_change_popup(const std::vector<double> &currentA, const std::vector<double> &currentB, const int current_delay, const double current_noise, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ARX_change_popup)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    ui->ARX_spinBox_count->blockSignals(true);

    //delay
    ui->ARX_delay->setValue(current_delay);

    //noise
    ui->ARX_noise->setValue(current_noise);

    //ile rzędów
    int countFromFile = std::max(currentA.size(), currentB.size());
    int finalCount = std::max(3, countFromFile);

    ui->ARX_spinBox_count->setMinimum(3);
    ui->ARX_spinBox_count->setValue(finalCount);

    updateListsCount(finalCount);

    updateListsCount(ui->ARX_spinBox_count->value());

    // A
    for (size_t i = 0; i < inputsA.size(); ++i) {
        if (i < currentA.size()) {
            inputsA[i]->setValue(currentA[i]);
        } else {
            inputsA[i]->setValue(0.0); // dopełnienie na KOŃCU
        }
    }

    // B
    for (size_t i = 0; i < inputsB.size(); ++i) {
        if (i < currentB.size()) {
            inputsB[i]->setValue(currentB[i]);
        } else {
            inputsB[i]->setValue(0.0);
        }
    }

    ui->ARX_spinBox_count->blockSignals(false);

    //połączenie sygnałów
    connect(ui->ARX_spinBox_count, QOverload<int>::of(&QSpinBox::valueChanged), this, &ARX_change_popup::updateListsCount);
}

ARX_change_popup::~ARX_change_popup()
{
    delete ui;
}

//przycisk anulujący wpisywanie danych i zamykajacy okno
void ARX_change_popup::on_btn_cancel_clicked()
{
    reject();
}

//przycisk zatwierający wpisanie danych
void ARX_change_popup::on_btn_save_clicked()
{
    std::vector<double> vecA;
    std::vector<double> vecB;

    for(auto spin : inputsA)
    {
        vecA.push_back(spin->value());
    }
    for(auto spin : inputsB)
    {
        vecB.push_back(spin->value());
    }
    int delay = static_cast<int>(ui->ARX_delay->value());
    double noise = ui->ARX_noise->value();

    emit updateArxData(vecA, vecB, delay, noise);

    accept();
}


void ARX_change_popup::updateListsCount(int count)
{
    updateInputList(count, ui->ARX_scrollAreaA, inputsA);
    updateInputList(count, ui->ARX_scrollAreaB, inputsB);
}

void ARX_change_popup::updateInputList(int count, QScrollArea* scrollArea, std::vector<QDoubleSpinBox*> &list)
{
    if (!scrollArea) return;

    QWidget *contents = scrollArea->widget();
    if (!contents) return; // Zabezpieczenie

    QVBoxLayout *innerLayout = qobject_cast<QVBoxLayout*>(contents->layout());

    if (!innerLayout) {
        innerLayout = new QVBoxLayout(contents);
        innerLayout->setAlignment(Qt::AlignTop);
    }

    int currentSize = list.size();

    // Dodawanie elementów
    if (count > currentSize)
    {
        for (int i = currentSize; i < count; ++i)
        {
            QDoubleSpinBox* spin = new QDoubleSpinBox(this);
            spin->setRange(-100.0, 100.0);
            spin->setDecimals(1);
            spin->setSingleStep(0.1);
            innerLayout->addWidget(spin);
            list.push_back(spin);
        }
    }
    // Usuwanie elementów
    else if (count < currentSize)
    {
        for (int i = currentSize; i > count; --i) {
            QDoubleSpinBox* spin = list.back();
            list.pop_back();
            innerLayout->removeWidget(spin);
            delete spin;
        }
    }
}
