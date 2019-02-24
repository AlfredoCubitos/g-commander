#include "hardwarewidget.h"
#include "ui_hardwarewidget.h"

#define SERIAL_INFO_REFRESH_DELAY  1000


HardwareWidget::HardwareWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HardwareWidget)
{
    ui->setupUi(this);

    createSerialIcon();

    ui->baudrateBox->addItem(QStringLiteral("115200"));
    ui->baudrateBox->addItem(QStringLiteral("9600"));

    fillSerialPortsSettings();

    m_serialInfoRefreshTimer = new QTimer(this);
    m_serialInfoRefreshTimer->setInterval(SERIAL_INFO_REFRESH_DELAY);

    connect(m_serialInfoRefreshTimer,&QTimer::timeout,this,&HardwareWidget::fillSerialPortsSettings);
    connect(ui->serialTogglebutton,&QPushButton::clicked,this,&HardwareWidget::onSerialToggleButtonClicked);
    connect(ui->grblSettingsButton,&QPushButton::clicked,this,&HardwareWidget::grblSettingsButtonPressed);

}

HardwareWidget::~HardwareWidget()
{
    delete ui;
}

void HardwareWidget::onGrblStatusUpdated(const GrblStatus *status){
    //Serial combo-boxes
    ui->portBox->setEnabled(!status->isStateOnline());
    ui->baudrateBox->setEnabled(!status->isStateOnline());

    //Serial toggle button
    ui->serialTogglebutton->setChecked(status->isStateOnline());

    //Status label
    ui->grblStateLabel->setText(status->getStateString());

    //Grbl settings button
    ui->grblSettingsButton->setEnabled(status->isStateKnown());

    //Serial refrash
    if(!status->isStateOnline() && !m_serialInfoRefreshTimer->isActive()){
        m_serialInfoRefreshTimer->start();
    }
    else if(status->isStateOnline() && m_serialInfoRefreshTimer->isActive()){
        m_serialInfoRefreshTimer->stop();
    }

}

void HardwareWidget::fillSerialPortsSettings(){
    QList<QSerialPortInfo> updatedAvailabledPortInfosList = QSerialPortInfo::availablePorts();

    //If new device added or removed
    if(updatedAvailabledPortInfosList.size() != m_serialPortsInfoList.size()){

        //Updated UI
        ui->portBox->hidePopup();
        ui->portBox->clear();
        foreach (const QSerialPortInfo &info, updatedAvailabledPortInfosList) {

            ui->portBox->addItem(info.portName());
        }

        //Save current device list
        m_serialPortsInfoList.swap(updatedAvailabledPortInfosList);
    }
}

void HardwareWidget::createSerialIcon(){
    QIcon serialIcon;
    QPixmap iconSerialConnected(":/serial-connected");
    serialIcon.addPixmap(iconSerialConnected,QIcon::Normal,QIcon::On);
    QPixmap iconSerialDisconnected(":/serial-disconnected");
    serialIcon.addPixmap(iconSerialDisconnected,QIcon::Normal,QIcon::Off);
    ui->serialTogglebutton->setIcon(serialIcon);
}

void HardwareWidget::onSerialToggleButtonClicked(bool isChecked){
    if(isChecked){
        emit serialSettingsUpdated(ui->portBox->currentText(),ui->baudrateBox->currentText().toInt());
    }
    emit toggleSerial();
}
