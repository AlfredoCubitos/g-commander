#include "monitorwidget.h"
#include "ui_monitorwidget.h"
#include "grblinstruction.h"

MonitorWidget::MonitorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MonitorWidget)
{
    ui->setupUi(this);

    history = new HistoryModel(this);

    ui->monitorView->setModel(history);

    connect(history,&HistoryModel::modelUpdated,ui->monitorView,&QTreeView::expand);
    connect(history,&HistoryModel::modelUpdated,ui->monitorView,&QTreeView::scrollToBottom);
    connect(ui->sendInstructionButton,&QPushButton::clicked,this,&MonitorWidget::onManualInstructionSendRequested);
    connect(ui->sendInstructionLineEdit,&QLineEdit::returnPressed,this,&MonitorWidget::onManualInstructionSendRequested);

}

void MonitorWidget::onGrblStatusUpdated(const GrblStatus *status){
    //Handle instruction fields
    ui->sendInstructionButton->setEnabled(status->isStateKnown());
    ui->sendInstructionLineEdit->setEnabled(status->isStateKnown());
    ui->monitorView->setEnabled(status->isStateKnown());
}

void MonitorWidget::onInstructionSentToGrbl(GrblInstruction instruction){
    history->onInstructionAcceptedInBoardCharBuffer(instruction);
}

void MonitorWidget::onOkReceived(GrblInstruction instruction){
    history->onOkReceived(instruction);
}

void MonitorWidget::onErrorReceived(GrblInstruction instruction, QString errorMessage){
    history->onErrorReceived(instruction,errorMessage);
}

void MonitorWidget::onFeedbackReceived(GrblInstruction instruction, QString feedbackMessage){
    history->onFeedbackReceived(instruction,feedbackMessage);
}

void MonitorWidget::onAlarmReceived(GrblInstruction instruction, QString alarmMessage){
    history->onAlarmReceived(instruction,alarmMessage);
}


void MonitorWidget::onBoardStartup(QString version, QList<GrblInstruction> instructionAtStartupList){
    history->onBoardStartup(version,instructionAtStartupList);
}

void MonitorWidget::onManualInstructionSendRequested(){
    GrblInstruction instruction(ui->sendInstructionLineEdit->text());
    instruction.forceBlocking();

    ui->sendInstructionLineEdit->clear();

    emit sendInstruction(instruction);
}

MonitorWidget::~MonitorWidget()
{
    delete ui;
}

