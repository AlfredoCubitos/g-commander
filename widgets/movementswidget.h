#ifndef MOVEMENTSWIDGET_H
#define MOVEMENTSWIDGET_H

#include <QWidget>
#include <grblinstruction.h>
#include "grblstatus.h"

namespace Ui {
class MovementsWidget;
}

class MovementsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovementsWidget(QWidget *parent = 0);
    ~MovementsWidget();

signals:
    void instructionToGrbl(GrblInstruction instruction);

public slots:
    void onGrblStatusUpdated(const GrblStatus *status);

private slots:

    void moveXForward();
    void moveXBackward();
    void moveYForward();
    void moveYBackward();
    void moveZForward();
    void moveZBackward();
    void gotoX0Y0();
    void gotoZ0();

    void sendIntruction(QString instructionString);

private:

    Ui::MovementsWidget *ui;
};

#endif // MOVEMENTSWIDGET_H
