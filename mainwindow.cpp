#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::update()
{
    //qDebug() << "On timer";
    port.write(":MEAS:CURR?\r\n");
    port.write(":MEAS:VOLT?\r\n");
}

void MainWindow::readyRead()
{
    double value = 0;
    bool isCurrent = false;

    QString tmp = port.readAll();

    tmp = tmp.left(tmp.length() - 1);

    if (tmp.at(tmp.length() - 1) == 'A')
        isCurrent = true;

    value = tmp.left(tmp.length() - 1).toDouble();

    if (isCurrent)
    {
        lastCurrent = value;

        if (lastCurrent > highestCurrent)
                highestCurrent = lastCurrent;

        return;
    } else
        if (value <= ui->sbCutVolts->value())
        {
            on_btnStop_clicked();

            QMessageBox msgBox;
            msgBox.setText("Measurement finished.");
            msgBox.setInformativeText(QString("Voltage %1 reached cut value %2. Result is: %3mAh").arg(value).arg(ui->sbCutVolts->value()).arg(ah));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();

            return;
        }

    lastVoltage = value;

    if (value > highestVolt)
            highestVolt = value;

    ah += ((lastCurrent * 1) / 60 / 60) * 1000;

    seriesVolts->append(ah, lastVoltage);
    seriesCurrent->append(ah, lastCurrent);

    axisX->setRange(-2, ah + 2);
    axisY->setRange(ui->sbCutVolts->value(), highestVolt + 0.5);
    axisY2->setRange(0, highestCurrent + 0.5);

    ui->lblResult->setText(QString("%1 mAh").arg(ah));

    timer->start();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ah = 0;
    highestVolt = 0;
    highestCurrent = 0;
    lastCurrent = 0;
    lastVoltage = 0;

    for (int i = 0; i < QSerialPortInfo::availablePorts().count(); i++)
        ui->cbPort->addItem(QSerialPortInfo::availablePorts().at(i).portName());

    timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    connect(&port, SIGNAL(readyRead()), this, SLOT(readyRead()));

    QChart *chart = new QChart();

    seriesVolts = new QLineSeries();
    seriesVolts->setName("Battery voltage");

    seriesCurrent = new QLineSeries();
    seriesCurrent->setName("Battery current");

    chart->addSeries(seriesVolts);
    chart->addSeries(seriesCurrent);

    chart->setTitle("Discharge chart");

    axisX = new QValueAxis;
    axisX->setTitleText("Capacity (mAh)");
    chart->addAxis(axisX, Qt::AlignBottom);
    seriesVolts->attachAxis(axisX);
    seriesCurrent->attachAxis(axisX);

    axisY = new QValueAxis;
    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("Voltage (V)");
    chart->addAxis(axisY, Qt::AlignLeft);
    seriesVolts->attachAxis(axisY);

    axisY2 = new QValueAxis;
    axisY2->setLabelFormat("%.2f");
    axisY2->setTitleText("Current (A)");
    chart->addAxis(axisY2, Qt::AlignRight);
    seriesCurrent->attachAxis(axisY2);

    chartView = new QChartView(chart);
    ui->verticalLayout->insertWidget(1, chartView);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnStart_clicked()
{
    ah = 0;
    highestVolt = 0;
    lastCurrent = 0;
    lastVoltage = 0;

    ui->lblResult->setText("0");

    seriesVolts->clear();
    seriesCurrent->clear();

    port.write(":INP 0\r\n");
    port.write(":CURR 0A\r\n");
    port.write(":FUNC CC\r\n");
    port.write(QString(":CURR %1A\r\n").arg(double(ui->sbCurrent->value()) / 1000).toStdString().c_str());
    port.write(":INP 1\r\n");

    timer->start();
    ui->btnStop->setEnabled(true);
    ui->btnStart->setEnabled(false);
}

void MainWindow::on_btnOpen_clicked()
{
    port.setPortName(ui->cbPort->currentText());
    port.setBaudRate(QSerialPort::Baud115200);
    port.setDataBits(QSerialPort::Data8);
    port.setParity(QSerialPort::NoParity);
    port.setStopBits(QSerialPort::OneStop);
    port.setFlowControl(QSerialPort::NoFlowControl);

    port.open(QSerialPort::ReadWrite);

    if (port.isOpen())
    {
        ui->btnOpen->setEnabled(false);
        ui->btnClose->setEnabled(true);
        ui->btnStart->setEnabled(true);
    }
}

void MainWindow::on_btnClose_clicked()
{
    on_btnStop_clicked();

    Sleep(1000);

    port.close();

    if (port.isOpen() == false)
    {
        ui->btnOpen->setEnabled(true);
        ui->btnClose->setEnabled(false);
        ui->btnStart->setEnabled(false);
    }
}

void MainWindow::on_btnStop_clicked()
{
    timer->stop();
    port.write(":INP 0\r\n");
    port.write(":CURR 0A\r\n");
    port.write(":FUNC CC\r\n");
    port.write(":INP 0\r\n");

    ui->btnStart->setEnabled(true);
    ui->btnStop->setEnabled(false);
}

void MainWindow::on_btnRequest_clicked()
{

}
