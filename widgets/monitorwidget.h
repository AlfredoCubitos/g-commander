#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QWidget>
#include "historymodel.h"
#include "grblstatus.h"
#include "grblinstruction.h"

namespace Ui {
class MonitorWidget;
}

class MonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorWidget(QWidget *parent = 0);
    ~MonitorWidget();

signals:
    void sendInstruction(GrblInstruction instruction);

public slots:
    void onBoardStartup(QString version, QList<GrblInstruction> instructionAtStartupList);

    void onGrblStatusUpdated(const GrblStatus *status);

    void onInstructionSentToGrbl(GrblInstruction instruction);

    void onOkReceived(GrblInstruction instruction);
    void onErrorReceived( GrblInstruction instruction, QString errorMessage);
    void onFeedbackReceived(GrblInstruction instruction, QString feedbackMessage);
    void onAlarmReceived(GrblInstruction instruction, QString alarmMessage);

private slots:
    void onManualInstructionSendRequested(void);

private:
    HistoryModel* history;
    Ui::MonitorWidget *ui;
};

#endif // MONITORWIDGET_H
