#ifndef GRBLERRORRECORDER_H
#define GRBLERRORRECORDER_H

#include <QObject>

#include "grblinstruction.h"

class GrblErrorRecorder : public QObject
{
    Q_OBJECT
public:
    explicit GrblErrorRecorder(QObject *parent = nullptr);

    QString getErrorSummary();
    int getErrorCount();

signals:

public slots:
    void clear();
    void addError(const GrblInstruction &instruction, const QString &errorString);

private:

    QStringList m_errorStringList;
};

#endif // GRBLERRORRECORDER_H
