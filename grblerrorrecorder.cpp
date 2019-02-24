#include "grblerrorrecorder.h"

GrblErrorRecorder::GrblErrorRecorder(QObject *parent) : QObject(parent){

}

QString GrblErrorRecorder::getErrorSummary(){
    return m_errorStringList.join('\n');
}

int GrblErrorRecorder::getErrorCount(){
    return m_errorStringList.size();
}

void GrblErrorRecorder::clear(){
    m_errorStringList.clear();
}

void GrblErrorRecorder::addError(GrblInstruction instruction, QString errorString){
    QString errorSummaryString("%1    %2"); //No need for a line return char, since instruction comes with a line return
    m_errorStringList.append(errorSummaryString.arg(instruction.getStringWithLineNumber(),errorString));
}
