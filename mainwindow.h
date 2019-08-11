#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtCharts>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void update();
    void readyRead();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnStart_clicked();

    void on_btnOpen_clicked();

    void on_btnClose_clicked();

    void on_btnStop_clicked();

    void on_btnRequest_clicked();

private:
    double ah;
    double highestVolt;
    double highestCurrent;
    double lastVoltage;
    double lastCurrent;

    QValueAxis *axisX;
    QValueAxis *axisY;
    QValueAxis *axisY2;

    QLineSeries *seriesVolts;
    QLineSeries *seriesCurrent;

    Ui::MainWindow *ui;

    QChartView *chartView;
    QTimer *timer;
    QSerialPort port;
};

#endif // MAINWINDOW_H
