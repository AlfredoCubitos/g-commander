#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QSignalMapper>
#include <QWidget>
#include <grblstatus.h>
#include <grblinstruction.h>

namespace Ui {
class ControlWidget;
}

class ControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControlWidget(QWidget *parent = 0);
    ~ControlWidget();
signals:
    void cmdReset();
    void cmdPause();
    void cmdResume();
    void sendInstruction(GrblInstruction instruction);

public slots:
    void onGrblStatusUpdated(const GrblStatus *status);

private slots:
    void craftInstruction(QString intructionString);

private:
    Ui::ControlWidget *ui;
    QSignalMapper* m_buttonCommandsMapper;
};

#endif // CONTROLWIDGET_H
