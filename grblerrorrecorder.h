#ifndef GRBLERRORRECORDER_H
#define GRBLERRORRECORDER_H

#include <QObject>

#include "grblinstruction.h"

class GrblErrorRecorder : public QObject
{
    Q_OBJECT
public:
    explicit GrblErrorRecorder(QObject *parent = 0);

    QString getErrorSummary();
    int getErrorCount();

signals:

public slots:
    void clear();
    void addError(GrblInstruction instruction, QString errorString);

private:

    QStringList m_errorStringList;
};

#endif // GRBLERRORRECORDER_H
