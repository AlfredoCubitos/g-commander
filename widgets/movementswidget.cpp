#include "movementswidget.h"
#include "ui_movementswidget.h"

MovementsWidget::MovementsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MovementsWidget)
{
    ui->setupUi(this);

    connect(ui->cmdXBackwardButton, &QPushButton::clicked, this, &MovementsWidget::moveXBackward);
    connect(ui->cmdXForwardButton, &QPushButton::clicked, this, &MovementsWidget::moveXForward);
    connect(ui->cmdYBackwardButton, &QPushButton::clicked, this, &MovementsWidget::moveYBackward);
    connect(ui->cmdYForwardButton, &QPushButton::clicked, this, &MovementsWidget::moveYForward);
    connect(ui->cmdZBackwardButton, &QPushButton::clicked, this, &MovementsWidget::moveZBackward);
    connect(ui->cmdZForwardButton, &QPushButton::clicked, this, &MovementsWidget::moveZForward);
    connect(ui->x0y0Button, &QPushButton::clicked, this, &MovementsWidget::gotoX0Y0);
    connect(ui->z0Button, &QPushButton::clicked, this, &MovementsWidget::gotoZ0);
}

MovementsWidget::~MovementsWidget()
{
    delete ui;
}

void MovementsWidget::onGrblStatusUpdated(const GrblStatus *status)
{
    setEnabled(status->isStateKnown());
}

void MovementsWidget::moveXForward()
{
    QString instructionString("G91 G0 X+%1");
    sendIntruction(instructionString.arg(ui->movementDistanceSpinBox->value()));
}

void MovementsWidget::moveXBackward()
{
    QString instructionString("G91 G0 X-%1");
    sendIntruction(instructionString.arg(ui->movementDistanceSpinBox->value()));
}

void MovementsWidget::moveYForward()
{
    QString instructionString("G91 G0 Y+%1");
    sendIntruction(instructionString.arg(ui->movementDistanceSpinBox->value()));
}

void MovementsWidget::moveYBackward()
{
    QString instructionString("G91 G0 Y-%1");
    sendIntruction(instructionString.arg(ui->movementDistanceSpinBox->value()));
}

void MovementsWidget::moveZForward()
{
    QString instructionString("G91 G0 Z+%1");
    sendIntruction(instructionString.arg(ui->movementDistanceSpinBox->value()));
}

void MovementsWidget::moveZBackward()
{
    QString instructionString("G91 G0 Z-%2");
    sendIntruction(instructionString.arg(ui->movementDistanceSpinBox->value()));
}

void MovementsWidget::gotoX0Y0()
{
    sendIntruction(QStringLiteral("G90 X0 Y0"));
}

void MovementsWidget::gotoZ0()
{
    sendIntruction(QStringLiteral("G90 Z0"));
}


void MovementsWidget::sendIntruction(QString instructionString)
{
    GrblInstruction instruction(instructionString);
    instruction.forceBlocking();
    emit instructionToGrbl(instruction);
}
