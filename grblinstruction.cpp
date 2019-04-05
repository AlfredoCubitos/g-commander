#include "grblinstruction.h"
#include "grbldefinitions.h"

#include <QStringList>

const char* GrblInstruction::s_blockingInstructionsList[] = {INSTRUCTIONS_BLOCKING};

uint32_t GrblInstruction::s_uidCounter = 0u;

GrblInstruction::GrblInstruction(QString instruction, int lineNumber):
    m_uid(s_uidCounter++),
    m_instructionBytes(instruction.toLatin1()),
    m_lineNumber(lineNumber)
{
    //Remove any Grbl realtime command
    m_instructionBytes.replace(QByteArrayLiteral(CMD_PAUSE_STRING),QByteArray());
    m_instructionBytes.replace(QByteArrayLiteral(CMD_RESUME_STRING),QByteArray());
    m_instructionBytes.replace(QByteArrayLiteral(CMD_STATUS_REQ_STRING),QByteArray());
    m_instructionBytes.replace(QByteArrayLiteral(CMD_SOFT_RESET_STRING),QByteArray());
    m_instructionBytes.replace(QByteArrayLiteral(CMD_SAFETY_DOOR),QByteArray());

    //If not already present, append END_OF_INSTRUCTION character
    if(!m_instructionBytes.isEmpty() && !m_instructionBytes.endsWith(END_OF_INSTRUCTION)){
        m_instructionBytes.append(END_OF_INSTRUCTION);
    }

    m_isBlocking = isInherentlyBlocking();
}

bool GrblInstruction::isInherentlyBlocking(){
    int blockingInstructionCount = sizeof(s_blockingInstructionsList)/sizeof(s_blockingInstructionsList[0]);
    for(int i = 0 ; i < blockingInstructionCount ; i++){
        if(m_instructionBytes.startsWith(s_blockingInstructionsList[i])){
            return true;
        }
    }
    return false;
}

void GrblInstruction::forceBlocking(){
    m_isBlocking = true;
}

QString GrblInstruction::getStringWithLineNumber() const{
    if(m_lineNumber > 0){
        QString returnString("Line %1 : %2");
        return returnString.arg(m_lineNumber).arg(QString::fromLatin1(m_instructionBytes));
    }
    else{
        return QString::fromLatin1(m_instructionBytes);
    }
}

bool GrblInstruction::isParameterFetch() const{
    return (m_instructionBytes == INST_GET_PARAMS"\n" || m_instructionBytes == INST_GET_PARAMS"\r");
}

void GrblInstruction::regenerate(){
    m_uid = s_uidCounter++;
}

bool operator!=(const GrblInstruction &i1, const GrblInstruction &i2){
    return i1.m_uid != i2.m_uid;
}

bool operator== (const GrblInstruction &i1, const GrblInstruction &i2){
    return i1.m_uid == i2.m_uid;
}
