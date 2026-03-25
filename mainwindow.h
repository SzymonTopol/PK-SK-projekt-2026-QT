#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "arx_change_popup.h"
#include "arx_change_limits.h"
#include "ServicesManager.h"

#include <QTimer>
#include <QtCharts>

//pliki dialogowe oraz zapis do pliku
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //ARX
    void on_btn_ARX_change_popup_clicked();
    void applyArxParamsChanged(const std::vector<double> &a, const std::vector<double>&b, int delay, double noise);
    void on_btn_ARX_change_popup_borders_clicked();
    void applyArxLimitsChanged(const double U_min, const double U_max, const double Y_min, const double Y_max);

    //PID
    void on_input_PID_k_editingFinished();
    void on_input_PID_Ti_editingFinished();
    void on_input_PID_Td_editingFinished();
    void on_comboBox_PID_error_currentIndexChanged(int index);
    void on_btn_intergral_reset_clicked();
    void updatePID();

    //GEN
    void on_checkBox_setValue_toggled(bool checked);
    void on_comboBox_GEN_function_currentIndexChanged(int index);

    void on_input_GEN_Amplitude_editingFinished();
    void on_input_GEN_offset_editingFinished();
    void on_input_GEN__T_editingFinished();
    void on_input_GEN_fill_editingFinished();

    void updateGEN();

    //SIM
    void on_btn_SIMULATION_start_clicked();
    void on_btn_SIMULATION_stop_clicked();
    void on_btn_SIMULATION_reset_clicked();
    void updateCharts();

    //slider oraz spinbox dla ms próbkowania
    void on_HorizontalSlider_ms_setup_valueChanged(int value);
    void on_spinBox_ms_setup_valueChanged(int arg1);

    //slider oraz spinbox dla okna czasowego
    void on_horizontalSlider_time_frame_valueChanged(int value);
    void on_spinBox_time_frame_valueChanged(int arg1);

    //zapis do pliku
    void saveToFile();
    void loadFromFile();

    //hard reset aplikacji
    void hardResetApp();

    //reczne skalowanie wykresow
    void on_pushButton_rescale_charts_clicked();

    void on_checkBox_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    ARX_change_popup *arxWindow = nullptr;
    arx_change_limits *limitsWindow = nullptr;

    int interval_ms;
    int time_window_s; // okno czasowe w sekundach
    double simulation_time_s; // czas symulacji w sekundach

    bool autoRescaleCharts = true;
    void setupCharts();
    void autoScaleYAxis(QValueAxis *axisX, QValueAxis *axisY, const QList<QLineSeries*> &seriesList);
    void updateSingleChart(QLineSeries* series, QValueAxis* axY, double time_s, double value);
    void styleFloatingLegend(QChart *chart, int legendHeight);
    void UpdateUIAfterLoad();

    // Metody do obsługi czasu
    int getMaxPointsInWindow() const;
    void updateAllXAxesRange(double current_time_s);

    // GŁÓWNY
    QLineSeries *seriesSetpoint = nullptr;
    QLineSeries *seriesOutputMain = nullptr;

    QValueAxis *axisX_Main = nullptr;
    QValueAxis *axisY_Main = nullptr;

    // STEROWANIA
    QLineSeries *seriesU = nullptr;
    QValueAxis *axisX_U = nullptr;
    QValueAxis *axisY_U = nullptr;

    // UCHYBU
    QLineSeries *seriesE = nullptr;
    QValueAxis *axisX_E = nullptr;
    QValueAxis *axisY_E = nullptr;

    // WYJŚCIA
    QLineSeries *seriesY = nullptr;
    QValueAxis *axisX_Y = nullptr;
    QValueAxis *axisY_Y = nullptr;

    // wykres pid
    QLineSeries *seriesP;
    QLineSeries *seriesI;
    QLineSeries *seriesD;
};
#endif // MAINWINDOW_H
