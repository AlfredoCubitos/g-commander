#include "coordinatedisplay.h"
#include "ui_coordinatedisplay.h"
#include "grbldefinitions.h"

CoordinateDisplay::CoordinateDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CoordinateDisplay)
{
    ui->setupUi(this);

    machPosText = ui->machPosLabel->text();
    workPosText = ui->workPosLabel->text();

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(ui->zeroXButton,&QPushButton::clicked,this,&CoordinateDisplay::onZeroXButtonPressed);
    connect(ui->zeroYButton,&QPushButton::clicked,this,&CoordinateDisplay::onZeroYButtonPressed);
    connect(ui->zeroZButton,&QPushButton::clicked,this,&CoordinateDisplay::onZeroZButtonPressed);
}

CoordinateDisplay::~CoordinateDisplay()
{
    delete ui;
}

void CoordinateDisplay::onGrblStatusUpdated(GrblStatus * const status)
{
    if(status->containsMachinePosition()){
        QVector3D machinePosition = status->getMachinePositionInGrblUnits();
        ui->machXlcd->display(machinePosition.x());
        ui->machYlcd->display(machinePosition.y());
        ui->machZlcd->display(machinePosition.z());
    }

    if(status->containsWorkPosition()){
        QVector3D workPosition = status->getWorkPositionInGrblUnits();
        ui->workXlcd->display(workPosition.x());
        ui->workYlcd->display(workPosition.y());
        ui->workZlcd->display(workPosition.z());
    }

    QString unitText = status->isUnitInches() ? QStringLiteral(" (in)") : QStringLiteral(" (mm)");
    ui->machPosLabel->setText(machPosText+unitText);
    ui->workPosLabel->setText(workPosText+unitText);
}

void CoordinateDisplay::onZeroXButtonPressed(){
    sendIntruction(QStringLiteral("G92 X0"));
}

void CoordinateDisplay::onZeroYButtonPressed(){
    sendIntruction(QStringLiteral("G92 Y0"));
}

void CoordinateDisplay::onZeroZButtonPressed(){
    sendIntruction(QStringLiteral("G92 Z0"));
}

void CoordinateDisplay::sendIntruction(QString instructionString)
{
    GrblInstruction instruction(instructionString);
    instruction.forceBlocking();
    emit instructionToGrbl(instruction);
}

