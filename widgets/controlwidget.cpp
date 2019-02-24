#include "controlwidget.h"
#include "ui_controlwidget.h"

#include "grbldefinitions.h"

ControlWidget::ControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlWidget)
{
    ui->setupUi(this);

    m_buttonCommandsMapper = new QSignalMapper(this);

    connect(this->ui->cmdResetButton,&QPushButton::clicked,this,&ControlWidget::cmdReset);

    connect(this->ui->cmdCheckButton,SIGNAL(clicked(bool)),m_buttonCommandsMapper,SLOT(map()));
    m_buttonCommandsMapper->setMapping(this->ui->cmdCheckButton,QStringLiteral(INST_TOGGLE_CHECK));
    connect(this->ui->cmdHomingButton,SIGNAL(clicked(bool)),m_buttonCommandsMapper,SLOT(map()));
    m_buttonCommandsMapper->setMapping(this->ui->cmdHomingButton,QStringLiteral(INST_HOMING));
    connect(this->ui->cmdKillAlarmButton,SIGNAL(clicked(bool)),m_buttonCommandsMapper,SLOT(map()));
    m_buttonCommandsMapper->setMapping(this->ui->cmdKillAlarmButton,QStringLiteral(INST_KILL_ALARM));

    connect(m_buttonCommandsMapper,SIGNAL(mapped(QString)),this,SLOT(craftInstruction(QString)));
}

ControlWidget::~ControlWidget()
{
    delete ui;
}

void ControlWidget::onGrblStatusUpdated(const GrblStatus *status){
    //Commands buttons
    ui->cmdResetButton->setEnabled(status->isStateOnline());
    ui->cmdHomingButton->setEnabled(status->isStateKnown() && !status->isStateCheck());
    ui->cmdKillAlarmButton->setEnabled(status->isStateAlarm());
    ui->cmdCheckButton->setEnabled(status->isStateNominal());
    ui->cmdHoldButton->setEnabled(status->isStateNominal() && !status->isStateCheck());

    //Handle multi-role buttons
    ui->cmdCheckButton->setChecked(status->getState() == GrblStatus::state_check);


    static QMetaObject::Connection holdButtonConnection;
    if(holdButtonConnection){
        disconnect(holdButtonConnection);
    }
    if(status->getState() == GrblStatus::state_hold || status->getState() == GrblStatus::state_door){
        ui->cmdHoldButton->setChecked(true);
        holdButtonConnection=connect(ui->cmdHoldButton,&QPushButton::clicked,this,&ControlWidget::cmdResume);
    }
    else{
        ui->cmdHoldButton->setChecked(false);
        holdButtonConnection=connect(ui->cmdHoldButton,&QPushButton::clicked,this,&ControlWidget::cmdPause);
    }

}

void ControlWidget::craftInstruction(QString intructionString){
    GrblInstruction instruction(intructionString);
    emit sendInstruction(instruction);

}
