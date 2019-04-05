#include "grblconfigurationdialog.h"
#include "ui_grblconfigurationdialog.h"
#include "grbldefinitions.h"

GrblConfigurationDialog::GrblConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GrblConfigurationDialog),
  m_lastSetParamInstruction("")
{
    ui->setupUi(this);


}

int GrblConfigurationDialog::exec(){
    sendGetParamsInstructions();
    return QDialog::exec();
}

void GrblConfigurationDialog::onParameterMapUpdated(QMap<int, GrblConfiguration> *parametersMap)
{
    static QMetaObject::Connection itemChangedConnection;

    if(itemChangedConnection){
        disconnect(itemChangedConnection);
    }

    ui->tableWidget->clearContents();

    m_parametersList = parametersMap->values();

    ui->tableWidget->setRowCount(m_parametersList.length());

    for(int i = 0 ; i < m_parametersList.size() ; i++){
        GrblConfiguration currentParam = m_parametersList.at(i);

        QTableWidgetItem *keyItem = new QTableWidgetItem(QString::number(currentParam.getKey()));
        keyItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->tableWidget->setItem(i,0,keyItem);

        QTableWidgetItem *valueItem = new QTableWidgetItem(currentParam.getValue().toString());
        valueItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
        ui->tableWidget->setItem(i,1,valueItem);

        QTableWidgetItem *captionItem = new QTableWidgetItem(currentParam.getCaption());
        captionItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        ui->tableWidget->setItem(i,2,captionItem);
    }

    itemChangedConnection = connect(ui->tableWidget,&QTableWidget::itemChanged,this,&GrblConfigurationDialog::onItemChanged);
}

void GrblConfigurationDialog::onItemChanged(QTableWidgetItem* item){
    if(item->column() != 1){
        return;
    }

    //Locate the item edited
    int index = item->row();
    if(0 <= index && index < m_parametersList.size()){
        GrblConfiguration param = m_parametersList.at(index);

        //Build matching instruction
        QString instructionString = QStringLiteral("$%1=%2").append(END_OF_INSTRUCTION);

        //Store it locally, so we can identify it later
        m_lastSetParamInstruction = GrblInstruction(instructionString.arg(param.getKey()).arg(item->data(Qt::EditRole).toString()));

        emit sendToGrbl(m_lastSetParamInstruction);
    }
}

void GrblConfigurationDialog::onInstructionOk(const GrblInstruction &instruction){
    if(instruction == m_lastSetParamInstruction){
        ui->iconLabel->setPixmap(QPixmap(":/monitor-ok"));
        ui->infoLabel->setText("Parameter edited successfully");
        sendGetParamsInstructions();
    }
}

void GrblConfigurationDialog::onInstructionError(const GrblInstruction &instruction, const QString &error){
    if(instruction == m_lastSetParamInstruction){
        ui->iconLabel->setPixmap(QPixmap(":/monitor-error"));
        ui->infoLabel->setText(QString("Could not edit parameter : %1").arg(error));
        sendGetParamsInstructions();
    }
}

void GrblConfigurationDialog::sendGetParamsInstructions(){
    sendToGrbl(GrblInstruction(INST_GET_PARAMS));
}

GrblConfigurationDialog::~GrblConfigurationDialog()
{
    delete ui;
}
