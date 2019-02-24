#ifndef HARDWAREWIDGET_H
#define HARDWAREWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QSerialPortInfo>

#include "grblstatus.h"

namespace Ui {
class HardwareWidget;
}

class HardwareWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HardwareWidget(QWidget *parent = 0);
    ~HardwareWidget();

signals:
    void grblSettingsButtonPressed();
    void serialSettingsUpdated(QString portName, qint32 baudRate);
    void toggleSerial();

public slots:
    void onGrblStatusUpdated(const GrblStatus *status);

private slots:
    void onSerialToggleButtonClicked(bool isChecked);

private:
    void fillSerialPortsSettings();
    void createSerialIcon();

    Ui::HardwareWidget *ui;
    QTimer* m_serialInfoRefreshTimer;
    QList<QSerialPortInfo> m_serialPortsInfoList;
};

#endif // HARDWAREWIDGET_H
