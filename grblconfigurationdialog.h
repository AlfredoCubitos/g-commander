#ifndef GRBLCONFIGURATIONDIALOG_H
#define GRBLCONFIGURATIONDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include "grblinstruction.h"
#include "grblconfiguration.h"

namespace Ui {
class GrblConfigurationDialog;
}

class GrblConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GrblConfigurationDialog(QWidget *parent = 0);

    ~GrblConfigurationDialog();

signals:
    void sendToGrbl(GrblInstruction instruction);

public slots:
    int exec();
    void onParameterMapUpdated(QMap<int,GrblConfiguration> *parametersMap);
    void onInstructionOk(GrblInstruction instruction);
    void onInstructionError(GrblInstruction instruction, QString error);

private slots:
    void onItemChanged(QTableWidgetItem* item);

private:
    void sendGetParamsInstructions(void);

    Ui::GrblConfigurationDialog *ui;

    QList<GrblConfiguration> m_parametersList;

    GrblInstruction m_lastSetParamInstruction;
};

#endif // GRBLCONFIGURATIONDIALOG_H
