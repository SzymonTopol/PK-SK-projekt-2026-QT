#include <QApplication>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusbar->hide();
    this->setMinimumSize(1200, 770);

    simulation_time_s = 0.0;

    int start_interval = 50;
    interval_ms = start_interval;
    ui->HorizontalSlider_ms_setup->setValue(start_interval);
    ui->spinBox_ms_setup->setValue(start_interval);
    ServicesManager::getInstance().setSimulationInterval(start_interval);

    int start_time_window = 10;
    time_window_s = start_time_window;
    ui->horizontalSlider_time_frame->setValue(start_time_window);
    ui->spinBox_time_frame->setValue(start_time_window);

    connect(&ServicesManager::getInstance(), &ServicesManager::SimulationUpdated, this, &MainWindow::updateCharts);

    setupCharts();

    connect(ui->input_setValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [](double val){
        ServicesManager::getInstance().setManualSetpoint(val);
    });

    //hard reset
    connect(ui->actionNowy_projekt, &QAction::triggered, this, &MainWindow::hardResetApp);
    //zapis do pliku
    connect(ui->actionZapisz_projekt, &QAction::triggered, this, &MainWindow::saveToFile);
    //odczyt z pliku
    connect(ui->actionWczytaj_projekt, &QAction::triggered, this, &MainWindow::loadFromFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//=====MODEL ARX=====

//przycisk otwierający okno popup z możliwością zmiany danych modelu ARX
void MainWindow::on_btn_ARX_change_popup_clicked()
{
    ServicesManager &sm = ServicesManager::getInstance();
    std::vector<double> a = sm.getArxA();
    std::vector<double> b = sm.getArxB();
    int k = sm.getArxDelay();
    double z = sm.getArxNoise();
    if(!arxWindow)
    {
        arxWindow = new ARX_change_popup(a, b, k, z, this);

        connect(arxWindow, &ARX_change_popup::updateArxData, this, &MainWindow::applyArxParamsChanged);
        connect(arxWindow, &QObject::destroyed, this, [this]()
        {
            arxWindow = nullptr;
        });

        arxWindow->show();
    }
    else
    {
        arxWindow->raise();
        arxWindow->activateWindow();
    }
}


//zaaplikowanie danych wczytanych z okna do modyfikacji ARX'a
void MainWindow::applyArxParamsChanged(const std::vector<double> &a, const std::vector<double>&b, int delay, double noise)
{
    ServicesManager &sm = ServicesManager::getInstance();
    sm.setArxParams(a, b, delay, noise);
    sm.applyParams();

    qDebug() << "Dane ARX załadowane!";
}

//limity
void MainWindow::on_btn_ARX_change_popup_borders_clicked()
{
    ServicesManager &sm = ServicesManager::getInstance();
    double U_MIN = sm.getBorderU().bottom;
    double U_MAX = sm.getBorderU().top;
    double Y_MIN = sm.getBorderY().bottom;
    double Y_MAX = sm.getBorderY().top;
    if(!limitsWindow)
    {
        limitsWindow = new arx_change_limits(this, U_MIN, U_MAX, Y_MIN, Y_MAX);

        connect(limitsWindow, &arx_change_limits::updateARXlimits, this, &MainWindow::applyArxLimitsChanged);
        connect(limitsWindow, &QObject::destroyed, this, [this]()
        {
            limitsWindow = nullptr;
        });

        limitsWindow->show();
    }
    else
    {
        limitsWindow->raise();
        limitsWindow->activateWindow();
    }
}

void MainWindow::applyArxLimitsChanged(const double U_min, const double U_max, const double Y_min, const double Y_max)
{
    Borders border_u = {U_min, U_max};
    Borders border_y = {Y_min, Y_max};

    ServicesManager &sm = ServicesManager::getInstance();

    sm.setArxBorders(border_u, border_y);

    qDebug() << "Zakresy ARX załadowane!";
    sm.applyParams();
}


//=====REGULATOR PID=====

void MainWindow::on_input_PID_k_editingFinished()
{

    updatePID();
}

void MainWindow::on_input_PID_Ti_editingFinished()
{
    updatePID();
}


void MainWindow::on_input_PID_Td_editingFinished()
{
    updatePID();
}

void MainWindow::on_comboBox_PID_error_currentIndexChanged(int index)
{
    updatePID();
}

//metoda pomocnicza
void MainWindow::updatePID()
{
    ServicesManager &sm = ServicesManager::getInstance();
    double p = ui->input_PID_k->value();
    double ti = ui->input_PID_Ti->value();
    double td = ui->input_PID_Td->value();
    RegulatorPID::LiczCalk liczcalk;
    if (ui->comboBox_PID_error->currentIndex() == 1)
        liczcalk = RegulatorPID::LiczCalk::Wew;
    else
        liczcalk = RegulatorPID::LiczCalk::Zew;
    sm.setPidParams(p, ti, td, liczcalk);
    sm.applyParams();    
}

void MainWindow::on_btn_intergral_reset_clicked()
{
    ServicesManager &sm = ServicesManager::getInstance();
    sm.resetPidIntegral();
    qDebug() << "Pamięć całki regulatora PID zresetowana.";
}

//=====GENERATOR=====

//checkbox do ustawienia stałej lub generowanej wartości zadanej
void MainWindow::on_checkBox_setValue_toggled(bool checked)
{
    //wartoś zadana stała
    ui->input_setValue->setEnabled(!checked);

    //wartość zadana generowana przez generator
    ui->comboBox_GEN_function->setEnabled(checked);
    ui->input_GEN_Amplitude->setEnabled(checked);
    ui->input_GEN__T->setEnabled(checked);
    ui->input_GEN_offset->setEnabled(checked);

    bool isSquare = (ui->comboBox_GEN_function->currentIndex() == 1);
    ui->input_GEN_fill->setEnabled(checked && isSquare);

    updateGEN();
}

void MainWindow::on_comboBox_GEN_function_currentIndexChanged(int index)
{
    bool isGeneratorOn = ui->checkBox_setValue->isChecked();

    if (isGeneratorOn && index == 1)
    {
        ui->input_GEN_fill->setEnabled(true);
    }
    else
    {
        ui->input_GEN_fill->setEnabled(false);
    }

    updateGEN();
}

//wartość zadana generowana przez generator
void MainWindow::on_input_GEN_Amplitude_editingFinished()
{
    updateGEN();
}

void MainWindow::on_input_GEN_offset_editingFinished()
{
    updateGEN();
}

void MainWindow::on_input_GEN__T_editingFinished()
{
    updateGEN();
}

void MainWindow::on_input_GEN_fill_editingFinished()
{
    updateGEN();
}

//metoda pomocnicza
void MainWindow::updateGEN()
{
    ServicesManager &sm = ServicesManager::getInstance();

    bool useGenerator = ui->checkBox_setValue->isChecked();
    sm.setUseGenerator(useGenerator);

    sm.setManualSetpoint(ui->input_setValue->value());

    if(useGenerator)
    {
        FunctionGenerator::FunctionType type = (ui->comboBox_GEN_function->currentIndex() == 0) ? FunctionGenerator::FunctionType::SIN : FunctionGenerator::FunctionType::SQUARE;

        double amp = ui->input_GEN_Amplitude->value();
        double period = ui->input_GEN__T->value();
        double offset = ui->input_GEN_offset->value();
        double fill = ui->input_GEN_fill->value();

        interval_ms = ui->spinBox_ms_setup->value();

        sm.setGeneratorParams(type, amp, period, interval_ms, offset, fill);
    }
    sm.applyParams();
}


//=====SYMULACJA=====

//slider czestotliwosci probkowania:
void MainWindow::on_HorizontalSlider_ms_setup_valueChanged(int value)
{
    interval_ms = value;

    ui->spinBox_ms_setup->blockSignals(true);
    ui->spinBox_ms_setup->setValue(value);
    ui->spinBox_ms_setup->blockSignals(false);

    ServicesManager::getInstance().setSimulationInterval(value);

    updateGEN();
}

//spinbox czestotliwosci probkowania:
void MainWindow::on_spinBox_ms_setup_valueChanged(int arg1)
{

    ui->HorizontalSlider_ms_setup->blockSignals(true);
    ui->HorizontalSlider_ms_setup->setValue(arg1);
    ui->HorizontalSlider_ms_setup->blockSignals(false);

    ServicesManager::getInstance().setSimulationInterval(arg1);

    updateGEN();
}

//slider okna czasowego:
void MainWindow::on_horizontalSlider_time_frame_valueChanged(int value)
{
    time_window_s = value;

    ui->spinBox_time_frame->blockSignals(true);
    ui->spinBox_time_frame->setValue(value);
    ui->spinBox_time_frame->blockSignals(false);

    updateAllXAxesRange(simulation_time_s);
}

//spinbox okna czasowego:
void MainWindow::on_spinBox_time_frame_valueChanged(int arg1)
{
    time_window_s = arg1;

    ui->horizontalSlider_time_frame->blockSignals(true);
    ui->horizontalSlider_time_frame->setValue(arg1);
    ui->horizontalSlider_time_frame->blockSignals(false);

    updateAllXAxesRange(simulation_time_s);
}

void MainWindow::on_btn_SIMULATION_start_clicked()
{
    ServicesManager &sm = ServicesManager::getInstance();

    if (ServicesManager::getInstance().isSimulationRunning()) return;
    updatePID();
    updateGEN();

    sm.setManualSetpoint(ui->input_setValue->value());
    sm.startSimulation();

    ui->btn_SIMULATION_start->setEnabled(false);
    ui->btn_SIMULATION_stop->setEnabled(true);

    qDebug() << "Symulacja uruchomiona";
}


void MainWindow::on_btn_SIMULATION_stop_clicked()
{
    ServicesManager::getInstance().stopSimulation();

    ui->btn_SIMULATION_start->setEnabled(true);
    ui->btn_SIMULATION_stop->setEnabled(false);

    qDebug() << "Symulacja zatrzymana";

}

void MainWindow::on_btn_SIMULATION_reset_clicked()
{
    // 1. Zatrzymanie symulacji, jeśli trwa
    on_btn_SIMULATION_stop_clicked();

    // 2. Reset backendu (logiki matematycznej)
    ServicesManager &sm = ServicesManager::getInstance();
    sm.resetSimulation();

    // 3. Reset czasu symulacji
    simulation_time_s = 0.0;

    // 4. Czyszczenie danych z wykresów (serii)
    if(seriesSetpoint) seriesSetpoint->clear();
    if(seriesOutputMain) seriesOutputMain->clear();

    if(seriesY) seriesY->clear();

    // Uchyb
    if(seriesE) seriesE->clear();

    // Składowe PID
    if(seriesP) seriesP->clear();
    if(seriesI) seriesI->clear();
    if(seriesD) seriesD->clear();

    // 5. Przywrócenie domyślnych zakresów osi (w sekundach)

    // Główny wykres
    if(axisX_Main) axisX_Main->setRange(0, time_window_s);
    if(axisY_Main) axisY_Main->setRange(-2, 2);

    // Wykres Y (środkowy)
    if(axisX_Y) axisX_Y->setRange(0, time_window_s);
    if(axisY_Y) axisY_Y->setRange(-2, 2);

    // Wykres Uchybu E
    if(axisX_E) axisX_E->setRange(0, time_window_s);
    if(axisY_E) axisY_E->setRange(-1, 1);

    // Wykres Sterowania (PID)
    if(axisX_U) axisX_U->setRange(0, time_window_s);
    if(axisY_U) axisY_U->setRange(-5, 5);

    qDebug() << "Symulacja i wszystkie wykresy zresetowane";
}

//hard reset w menu na pasku
void MainWindow::hardResetApp()
{
    on_btn_SIMULATION_reset_clicked();
    on_btn_intergral_reset_clicked();
    ServicesManager &sm = ServicesManager::getInstance();
    sm.hardResetSimulation();
    sm.setArxParams({0.0}, {0.0}, 1.0, 0.0);

    simulation_time_s = 0.0;

    ui->input_setValue->setValue(0.0);
    ui->input_PID_k->setValue(0.0);
    ui->input_PID_Ti->setValue(0.0);
    ui->input_PID_Td->setValue(0.0);
    ui->comboBox_PID_error->setCurrentIndex(0);
    ui->checkBox_setValue->setChecked(false);
    ui->comboBox_GEN_function->setCurrentIndex(0);
    ui->input_GEN_Amplitude->setValue(0.0);
    ui->input_GEN__T->setValue(0.0);
    ui->input_GEN_offset->setValue(0.0);
    ui->input_GEN_fill->setValue(0.0);
    ui->HorizontalSlider_ms_setup->setValue(50.0);
    ui->horizontalSlider_time_frame->setValue(10);
    ui->spinBox_time_frame->setValue(10);
    time_window_s = 10;
}

// Obliczenie maksymalnej liczby punktów w oknie czasowym
int MainWindow::getMaxPointsInWindow() const
{
    const double MSTOSEC = 1000.0;

    double current_ms = ServicesManager::getInstance().getGenSampleMs();

    return (time_window_s * MSTOSEC) / current_ms;
}

// Aktualizacja zakresów osi X dla wszystkich wykresów
void MainWindow::updateAllXAxesRange(double current_time_s)
{
    double minX, maxX;

    if (current_time_s > time_window_s) {
        minX = current_time_s - time_window_s;
        maxX = current_time_s;
    } else {
        minX = 0;
        maxX = time_window_s;
    }

    if (axisX_Main) axisX_Main->setRange(minX, maxX);
    if (axisX_Y) axisX_Y->setRange(minX, maxX);
    if (axisX_E) axisX_E->setRange(minX, maxX);
    if (axisX_U) axisX_U->setRange(minX, maxX);
}

void MainWindow::autoScaleYAxis(QValueAxis *axisX, QValueAxis *axisY, const QList<QLineSeries*> &seriesList)
{
    if (!autoRescaleCharts) return;
    if (!axisX || !axisY || seriesList.isEmpty()) return;

    double xMin = axisX->min();
    double xMax = axisX->max();

    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    bool foundAnyPoint = false;

    for (QLineSeries *series : seriesList) {
        if (!series) continue;

        int count = series->count();

        for (int i = 0; i < count; ++i) {
            const QPointF &p = series->at(i);

            double x = p.x();

            if (x < xMin) continue;
            if (x > xMax) break;

            double y = p.y();
            if (y < yMin) yMin = y;
            if (y > yMax) yMax = y;
            foundAnyPoint = true;
        }
    }

    if (!foundAnyPoint) return;

    if (qAbs(yMax - yMin) < 0.0001) {
        yMax += 1.0;
        yMin -= 1.0;
    }

    double margin = (yMax - yMin) * 0.1;
    if (margin == 0) margin = 0.1;

    axisY->setRange(yMin - margin, yMax + margin);
}

void MainWindow::updateCharts()
{
    ServicesManager &sm = ServicesManager::getInstance();
    const auto& data = sm.getSimulationData();
    const double MSTOSEC = 1000.0;

    if(data.empty()) return;

    const auto& lastPoint = data.back();
    int current_interval_ms = sm.getGenSampleMs();

    simulation_time_s += current_interval_ms / MSTOSEC;
    double time_s = simulation_time_s;

    int maxPoints = getMaxPointsInWindow() * 2;

    // --- Dodawanie punktów ---
    seriesSetpoint->append(time_s, lastPoint.w);
    seriesOutputMain->append(time_s, lastPoint.y);
    
    // PID
    seriesP->append(time_s, lastPoint.pid_values.p);
    seriesI->append(time_s, lastPoint.pid_values.i);
    seriesD->append(time_s, lastPoint.pid_values.d);

    // Inne
    seriesY->append(time_s, lastPoint.y);
    seriesE->append(time_s, lastPoint.e);

    // --- Usuwanie starych punktów (optymalizacja pamięci) ---
    if(seriesSetpoint->count() > maxPoints) {
        seriesSetpoint->removePoints(0, 1);
        seriesOutputMain->removePoints(0, 1);
    }
    if(seriesP->count() > maxPoints) {
        seriesP->removePoints(0, 1);
        seriesI->removePoints(0, 1);
        seriesD->removePoints(0, 1);
    }
    if(seriesY->count() > maxPoints) {
        seriesY->removePoints(0, 1);
    }
    if(seriesE->count() > maxPoints) {
        seriesE->removePoints(0, 1);
    }

    // --- Aktualizacja osi X (przesuwanie okna) ---
    updateAllXAxesRange(time_s);

    // --- INTELIGENTNE SKALOWANIE Y ---
    // Wywołujemy dopiero po aktualizacji osi X i dodaniu punktów!
    
    // 1. Wykres główny (zawiera dwie serie: Setpoint i Output)
    autoScaleYAxis(axisX_Main, axisY_Main, {seriesSetpoint, seriesOutputMain});

    // 2. Wykres Y (sama seria Y)
    autoScaleYAxis(axisX_Y, axisY_Y, {seriesY});

    // 3. Wykres E (uchyb)
    autoScaleYAxis(axisX_E, axisY_E, {seriesE});

    // 4. Wykres PID (trzy serie)
    autoScaleYAxis(axisX_U, axisY_U, {seriesP, seriesI, seriesD});
}

//===== WYKRESY =====
void MainWindow::updateSingleChart(QLineSeries* series, QValueAxis* axY, double time_s, double value)
{
    if(!series || !axY) return;

    series->append(time_s, value);

    int maxPoints = getMaxPointsInWindow() * 2;
    if (series->count() > maxPoints) {
        series->removePoints(0, 1);
    }

    double currentMin = axY->min();
    double currentMax = axY->max();
    double margin = 0.1;

    bool rangeChanged = false;

    // Jeśli wartość wystrzeliła w górę
    if (value >= currentMax) {
        currentMax = value + margin;
        rangeChanged = true;
    }
    // Jeśli wartość spadła w dół
    if (value <= currentMin) {
        currentMin = value - margin;
        rangeChanged = true;
    }

    if (rangeChanged) {
        axY->setRange(currentMin, currentMax);
    }
}

void MainWindow::styleFloatingLegend(QChart *chart, int legendHeight)
{
    QLegend *legend = chart->legend();
    legend->setVisible(true);
    legend->detachFromChart();

    legend->setBackgroundVisible(true);
    legend->setBrush(QBrush(QColor(255, 255, 255, 210)));
    legend->setPen(QPen(Qt::darkGray, 1));
    legend->setFont(QFont("Arial", 8));

    double legendWidth = 120;
    double margin = 8;

    auto updateLegendPosition = [legend, legendWidth, legendHeight, margin](const QRectF &plotArea) {
        if (plotArea.isEmpty()) return;

        double x = plotArea.x() + margin;
        double y = plotArea.y() + margin;

        legend->setGeometry(QRectF(x, y, legendWidth, legendHeight));
    };

    connect(chart, &QChart::plotAreaChanged, legend, updateLegendPosition);

    QTimer::singleShot(0, [chart, updateLegendPosition]() {
        updateLegendPosition(chart->plotArea());
    });
}

void MainWindow::setupCharts()
{
    // ==========================================
    // 1. WYKRES GŁÓWNY (Zadana w + Wyjście y)
    // ==========================================
    seriesSetpoint = new QLineSeries();
    seriesSetpoint->setName("Zadana (w)");
    seriesSetpoint->setPen(QPen(Qt::darkGray, 1));

    seriesOutputMain = new QLineSeries();
    seriesOutputMain->setName("Wyjście (y)");
    seriesOutputMain->setPen(QPen(Qt::blue, 1));

    QChart *chartMain = new QChart();
    chartMain->addSeries(seriesSetpoint);
    chartMain->addSeries(seriesOutputMain);
    chartMain->setMargins(QMargins(0, 0, 0, 0));
    chartMain->layout()->setContentsMargins(0, 0, 0, 0);
    chartMain->setBackgroundRoundness(0);

    axisX_Main = new QValueAxis();
    axisX_Main->setRange(0, time_window_s);
    axisX_Main->setLabelFormat("%.1f");
    axisX_Main->setTitleText("Czas [s]");
    axisX_Main->setTickCount(6);
    chartMain->addAxis(axisX_Main, Qt::AlignBottom);
    seriesSetpoint->attachAxis(axisX_Main);
    seriesOutputMain->attachAxis(axisX_Main);

    axisY_Main = new QValueAxis();
    axisY_Main->setRange(-2, 2);
    axisY_Main->setLabelFormat("%.1f");
    axisY_Main->setTickCount(5);
    chartMain->addAxis(axisY_Main, Qt::AlignLeft);
    seriesSetpoint->attachAxis(axisY_Main);
    seriesOutputMain->attachAxis(axisY_Main);

    ui->graphicsView_main->setChart(chartMain);
    ui->graphicsView_main->setRenderHint(QPainter:: Antialiasing);
    styleFloatingLegend(chartMain, 60);

    // ==========================================
    // 2. WYKRES ŚRODKOWY (Samo wyjście y)
    // ==========================================
    seriesY = new QLineSeries();
    seriesY->setName("Wyjście (y)");
    seriesY->setPen(QPen(Qt::blue, 1));

    QChart *chartY = new QChart();
    chartY->addSeries(seriesY);
    chartY->setMargins(QMargins(0, 0, 0, 0));
    chartY->layout()->setContentsMargins(0, 0, 0, 0);
    chartY->setBackgroundRoundness(0);

    axisX_Y = new QValueAxis();
    axisX_Y->setRange(0, time_window_s);
    axisX_Y->setLabelFormat("%.1f");
    axisX_Y->setTitleText("Czas [s]");
    axisX_Y->setTickCount(6);
    chartY->addAxis(axisX_Y, Qt::AlignBottom);
    seriesY->attachAxis(axisX_Y);

    axisY_Y = new QValueAxis();
    axisY_Y->setRange(-2, 2);
    axisY_Y->setLabelFormat("%.1f");
    axisY_Y->setTickCount(5);
    chartY->addAxis(axisY_Y, Qt:: AlignLeft);
    seriesY->attachAxis(axisY_Y);

    ui->graphicsView_2->setChart(chartY);
    ui->graphicsView_2->setRenderHint(QPainter::Antialiasing);
    styleFloatingLegend(chartY, 40);

    // ==========================================
    // 3. UCHYB (e)
    // ==========================================
    seriesE = new QLineSeries();
    seriesE->setName("Uchyb (e)");
    seriesE->setPen(QPen(Qt::darkGreen, 1));

    QChart *chartE = new QChart();
    chartE->addSeries(seriesE);
    chartE->setMargins(QMargins(0, 0, 0, 0));
    chartE->layout()->setContentsMargins(0, 0, 0, 0);
    chartE->setBackgroundRoundness(0);

    axisX_E = new QValueAxis();
    axisX_E->setRange(0, time_window_s);
    axisX_E->setLabelFormat("%.1f");
    axisX_E->setTitleText("Czas [s]");
    axisX_E->setTickCount(6);
    chartE->addAxis(axisX_E, Qt::AlignBottom);
    seriesE->attachAxis(axisX_E);

    axisY_E = new QValueAxis();
    axisY_E->setRange(-0.5, 0.5);
    axisY_E->setLabelFormat("%.1f");
    axisY_E->setTickCount(5);
    chartE->addAxis(axisY_E, Qt:: AlignLeft);
    seriesE->attachAxis(axisY_E);

    ui->graphicsView_3->setChart(chartE);
    ui->graphicsView_3->setRenderHint(QPainter:: Antialiasing);
    styleFloatingLegend(chartE, 40);

    // ==========================================
    // 4. STEROWANIE (P + I + D)
    // ==========================================
    seriesP = new QLineSeries();
    seriesP->setName("P");
    seriesP->setPen(QPen(Qt::red, 1));

    seriesI = new QLineSeries();
    seriesI->setName("I");
    seriesI->setPen(QPen(Qt::magenta, 1));

    seriesD = new QLineSeries();
    seriesD->setName("D");
    seriesD->setPen(QPen(Qt::cyan, 1));

    QChart *chartU = new QChart();
    chartU->addSeries(seriesP);
    chartU->addSeries(seriesI);
    chartU->addSeries(seriesD);
    chartU->setMargins(QMargins(0, 0, 0, 0));
    chartU->layout()->setContentsMargins(0, 0, 0, 0);
    chartU->setBackgroundRoundness(0);

    axisX_U = new QValueAxis();
    axisX_U->setRange(0, time_window_s);
    axisX_U->setLabelFormat("%.1f");
    axisX_U->setTitleText("Czas [s]");
    axisX_U->setTickCount(6);
    chartU->addAxis(axisX_U, Qt::AlignBottom);
    seriesP->attachAxis(axisX_U);
    seriesI->attachAxis(axisX_U);
    seriesD->attachAxis(axisX_U);

    axisY_U = new QValueAxis();
    axisY_U->setRange(-5, 5);
    axisY_U->setLabelFormat("%.1f");
    axisY_U->setTickCount(5);
    chartU->addAxis(axisY_U, Qt::AlignLeft);
    seriesP->attachAxis(axisY_U);
    seriesI->attachAxis(axisY_U);
    seriesD->attachAxis(axisY_U);

    ui->graphicsView_4->setChart(chartU);
    ui->graphicsView_4->setRenderHint(QPainter::Antialiasing);
    styleFloatingLegend(chartU, 40);
}

//zapis do pliku

void MainWindow::saveToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz projekt", "", "Pliki JSON (*.json)");

    if(fileName.isEmpty()) return;
    if(!fileName.endsWith(".json", Qt::CaseInsensitive)) fileName += ".json";

    if(ServicesManager::getInstance().saveState(fileName))
    {
        ui->statusbar->showMessage("Zapisano: " + fileName);
    }
    else
    {
        QMessageBox::warning(this, "Błąd", "Nie udało się zapisać pliku.");
    }
}

void MainWindow::loadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Wczytaj projekt", "", "Pliki JSON (*.json)");
    if(fileName.isEmpty()) return;

    if(ServicesManager::getInstance().loadState(fileName))
    {
        UpdateUIAfterLoad();

        ui->statusbar->showMessage("Wczytano: " + fileName);
    }
    else
    {
        QMessageBox::warning(this, "Błąd", "Nie udało się wczytać pliku.");
    }
}

void MainWindow::UpdateUIAfterLoad()
{
    ServicesManager &sm = ServicesManager::getInstance();

    // Blokujemy sygnały, żeby zmiana wartości w kodzie nie wyzwalała zbędnych eventów
    const bool oldStateCheck = ui->checkBox_setValue->blockSignals(true);
    const bool oldStateComboGen = ui->comboBox_GEN_function->blockSignals(true);
    const bool oldStateComboPID = ui->comboBox_PID_error->blockSignals(true);

    ui->input_PID_k->blockSignals(true);
    ui->input_PID_Ti->blockSignals(true);
    ui->input_PID_Td->blockSignals(true);

    ui->input_GEN_Amplitude->blockSignals(true);
    ui->input_GEN__T->blockSignals(true);
    ui->input_GEN_offset->blockSignals(true);
    ui->input_GEN_fill->blockSignals(true);

    // PID
    ui->input_PID_k->setValue(sm.getPidP());
    ui->input_PID_Ti->setValue(sm.getPidTi());
    ui->input_PID_Td->setValue(sm.getPidTd());

    // Generator / Zadana
    if (sm.getPidMethod() == RegulatorPID::LiczCalk::Wew)
        ui->comboBox_PID_error->setCurrentIndex(1);
    else
        ui->comboBox_PID_error->setCurrentIndex(0);

    bool useGen = sm.getUseGenerator();
    ui->checkBox_setValue->setChecked(useGen);

    // Odświeżenie stanu enabled/disabled pól
    ui->input_setValue->setEnabled(!useGen);
    ui->comboBox_GEN_function->setEnabled(useGen);
    ui->input_GEN_Amplitude->setEnabled(useGen);
    ui->input_GEN__T->setEnabled(useGen);
    ui->input_GEN_offset->setEnabled(useGen);

    if (sm.getGenType() == FunctionGenerator::FunctionType::SIN)
    {
        ui->comboBox_GEN_function->setCurrentIndex(0);
        ui->input_GEN_fill->setEnabled(false);
    }
    else
    {
        ui->comboBox_GEN_function->setCurrentIndex(1);
        ui->input_GEN_fill->setEnabled(true);
    }

    ui->input_GEN_Amplitude->setValue(sm.getGenAmp());
    ui->input_GEN__T->setValue(sm.getGenFreq());
    ui->input_GEN_offset->setValue(sm.getGenOffset());
    ui->input_GEN_fill->setValue(sm.getGenfill());

    int sampleMs = sm.getGenSampleMs();

    ui->spinBox_ms_setup->blockSignals(true);
    ui->HorizontalSlider_ms_setup->blockSignals(true);

    ui->spinBox_ms_setup->setValue(sampleMs);
    ui->HorizontalSlider_ms_setup->setValue(sampleMs);
    interval_ms = sampleMs;

    ui->spinBox_ms_setup->blockSignals(false);
    ui->HorizontalSlider_ms_setup->blockSignals(false);

    ui->checkBox_setValue->blockSignals(oldStateCheck);
    ui->comboBox_GEN_function->blockSignals(oldStateComboGen);
    ui->comboBox_PID_error->blockSignals(oldStateComboPID);

    ui->input_PID_k->blockSignals(false);
    ui->input_PID_Ti->blockSignals(false);
    ui->input_PID_Td->blockSignals(false);

    ui->input_GEN_Amplitude->blockSignals(false);
    ui->input_GEN__T->blockSignals(false);
    ui->input_GEN_offset->blockSignals(false);
    ui->input_GEN_fill->blockSignals(false);

    sm.WypiszDane();
}


void MainWindow::on_pushButton_rescale_charts_clicked()
{
    if(axisY_Main) axisY_Main->setRange(-2, 2);
    if(axisY_Y) axisY_Y->setRange(-2, 2);
    if(axisY_E) axisY_E->setRange(-1, 1);
    if(axisY_U) axisY_U->setRange(-5, 5);
    
    qDebug() << "Skale Y wykresów zresetowane";
}


void MainWindow::on_checkBox_toggled(bool checked)
{
    autoRescaleCharts = checked;
}

