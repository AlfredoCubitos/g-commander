#ifndef COORDINATEDISPLAY_H
#define COORDINATEDISPLAY_H

#include <QWidget>
#include "grblstatus.h"
#include "grblinstruction.h"
#include "grblconfiguration.h"

namespace Ui {
class CoordinateDisplay;
}

class CoordinateDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit CoordinateDisplay(QWidget *parent = 0);
    ~CoordinateDisplay();

signals:
    void instructionToGrbl(GrblInstruction instruction);

public slots:
    void onGrblStatusUpdated(GrblStatus* const status);

private slots:
    void onZeroXButtonPressed();
    void onZeroYButtonPressed();
    void onZeroZButtonPressed();

private:
    Ui::CoordinateDisplay *ui;
    void sendIntruction(QString instructionString);

    QString machPosText;
    QString workPosText;
};

#endif // COORDINATEDISPLAY_H
