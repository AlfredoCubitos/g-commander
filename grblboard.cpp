#include "grblboard.h"
#include "grbldefinitions.h"

#define DEFAULT_STATUS_REQUEST_INTERVAL 250


const QMap<int,QString> GrblBoard::errorTranslationMap = GrblBoard::generateErrorTranslationMap();

GrblBoard::GrblBoard(QObject *parent) :
    QObject(parent),
    m_status(false)
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::readyRead, this, &GrblBoard::onSerialDataAvailable);

    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(DEFAULT_STATUS_REQUEST_INTERVAL);
    connect(m_statusTimer,&QTimer::timeout,this,&GrblBoard::rtCmdRequestStatus);

    //Build instruction to send at startup list
    m_startupInstructionList.append(GrblInstruction(QStringLiteral(INST_GET_PARAMS)));
}

void GrblBoard::setSerialSettings(const QString &portName, const qint32 &baudRate){
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
}

void GrblBoard::toggleSerial(){
    if(m_serialPort->isOpen()){
        m_serialPort->close();
        m_statusTimer->stop();
        m_status = GrblStatus(false);
    }
    else if(m_serialPort->open(QSerialPort::ReadWrite)){
        m_status = GrblStatus(true);
        rtCmdSoftReset();     //Immediately performs a soft reset so the board is in a known state
    }

    emit statusUpdated(&m_status);
}

const GrblStatus *GrblBoard::getLastStatus(void) const{
    return &m_status;
}

QMap<int,GrblConfiguration> *GrblBoard::getParametersMap(void){
    return &m_parametersMapComplete;
}

void GrblBoard::setStatusRequestInterval(const int &interval){
    m_statusTimer->setInterval( interval);
}

void GrblBoard::onSerialDataAvailable(){
    m_bufferFromBoard.append(m_serialPort->readAll());

    while(m_bufferFromBoard.contains(LINE_SEPARATOR_STRING)){
        //Locate next line separator, marking end of the current line
        int lineSeparatorIndex = m_bufferFromBoard.indexOf(LINE_SEPARATOR_STRING,0);

        //We got a complete line to parse, extract it from buffer
        QString line = QString::fromLatin1(m_bufferFromBoard.left(lineSeparatorIndex));
        m_bufferFromBoard.remove(0,lineSeparatorIndex+LINE_SEPARATOR_LENGTH);

        //Get the instruction currently executed by the board
        GrblInstruction relatedInstruction = m_boardCharBuffer.isEmpty() ? GrblInstruction("") : m_boardCharBuffer.first();

        //Ok
        if(line == QStringLiteral(RESPONSE_OK)){
            if(!m_boardCharBuffer.isEmpty())m_boardCharBuffer.removeFirst();    //Instruction processed, not in char buffer any more

            //If received "ok" for a parameter fetch instruction, swap the parameter maps to make the new one available
            if(relatedInstruction.isParameterFetch()){
                m_parametersMapComplete.swap(m_parametersMapBeingFilled);
                m_parametersMapBeingFilled.clear();
                emit parametersMapUpdated(&m_parametersMapComplete);
            }

            emit ok(relatedInstruction);
        }

        //Error message
        else if(line.startsWith(RESPONSE_ERROR)){
            if(!m_boardCharBuffer.isEmpty())m_boardCharBuffer.removeFirst();    //Instruction processed, not in char buffer any more
            addErrorTranslation(&line);
            emit error(relatedInstruction,line);
        }

        //Alarm message
        else if(line.startsWith(RESPONSE_ALARM)){
            emit alarm(relatedInstruction,line);
        }

        //Status message
        else if(line.startsWith(RESPONSE_STATUS_START) && line.endsWith(RESPONSE_STATUS_END)){
            bool isBoardReportingInches = m_parametersMapComplete.value(GRBL_PARAM_REPORT_INCHES).getValue().toBool();

            m_status = GrblStatus(line,isBoardReportingInches,&m_status);

            emit statusUpdated(&m_status);
        }

        //Feedback message
        else if(line.startsWith(RESPONSE_FEEDBACK_START) && line.endsWith(RESPONSE_FEEDBACK_END)){
            emit feedback(relatedInstruction,line);
        }

        //Startup message
        else if(line.startsWith(VERSION_STRING)){
            m_boardCharBuffer.clear();      //Board char buffer emptied by reset

            //Request status and start status timer
            rtCmdRequestStatus();
            m_statusTimer->start();

            //Build startup instructions
            for(int i = 0 ; i < m_startupInstructionList.size() ; i++){
                m_startupInstructionList[i].regenerate();
            }

            emit boardStartup(line,m_startupInstructionList);

            //Give the board some time to send us messages, then send startup instructions
            QTimer::singleShot(100,this,&GrblBoard::sendStartupInstructions);
        }

        //Parameter value
        else if(GrblConfiguration::isAValidParameter(line)){
            GrblConfiguration param = GrblConfiguration::fromString(line);

            m_parametersMapBeingFilled.insert(param.getKey(),param);

            emit text(relatedInstruction,line);
        }

        //else it is just text
        else if(!line.simplified().isEmpty()){
            emit text(relatedInstruction,line);
        }
    }
}


void GrblBoard::sendInstruction(const GrblInstruction &instruction){
    //If serial link not opened, or a blocking instruction is in buffer, reject instruction
    if(!m_serialPort->isOpen() || isBlockingInstructionInBuffer()){
        return;
    }

    //if instruction would not fit in board rx buffer, reject it
    if(instruction.getLength() > getAvailableSpaceInCharBuffer()){
        return;
    }

    //Send instruction and keep track of its place in rx buffer of the board
    if(m_serialPort->write(instruction.getBytes()) > 0){
        m_boardCharBuffer.append(instruction);
        emit instructionSent(instruction);
    }

    return;
}

void GrblBoard::sendStartupInstructions(void){
    foreach(GrblInstruction instruction,m_startupInstructionList){
        sendInstruction(instruction);
    }
}

bool GrblBoard::isBlockingInstructionInBuffer(void){
    foreach(GrblInstruction inst,  m_boardCharBuffer){
        if(inst.isBlocking()){
            return true;
        }
    }
    return false;
}


int GrblBoard::getAvailableSpaceInCharBuffer(void){
    int freeSizeInBoardRxBuffer = BOARD_RX_BUFFER_SIZE;
    foreach(GrblInstruction inst, m_boardCharBuffer){
        freeSizeInBoardRxBuffer -= inst.getLength();
    }
    return freeSizeInBoardRxBuffer;
}



void GrblBoard::addErrorTranslation(QString *errorString){
    static const QRegularExpression errorIdRegExp = QRegularExpression(GRBL_ERR_REGEXP);

    QRegularExpressionMatch match = errorIdRegExp.match(errorString);
    if(match.hasMatch()){
        int id = match.captured("id").toInt();
        if(errorTranslationMap.contains(id)){
            errorString->append(" : ");
            errorString->append(errorTranslationMap.value(id));
        }
    }
}

QMap<int, QString> GrblBoard::generateErrorTranslationMap(){
    QMap<int,QString> errorTranslationMap;
    errorTranslationMap.insert(23,GRBL_ERR_23);
    errorTranslationMap.insert(24,GRBL_ERR_24);
    errorTranslationMap.insert(25,GRBL_ERR_25);
    errorTranslationMap.insert(26,GRBL_ERR_26);
    errorTranslationMap.insert(27,GRBL_ERR_27);
    errorTranslationMap.insert(28,GRBL_ERR_28);
    errorTranslationMap.insert(29,GRBL_ERR_29);
    errorTranslationMap.insert(30,GRBL_ERR_30);
    errorTranslationMap.insert(31,GRBL_ERR_31);
    errorTranslationMap.insert(32,GRBL_ERR_32);
    errorTranslationMap.insert(33,GRBL_ERR_33);
    errorTranslationMap.insert(34,GRBL_ERR_34);
    errorTranslationMap.insert(35,GRBL_ERR_35);
    errorTranslationMap.insert(36,GRBL_ERR_36);
    errorTranslationMap.insert(37,GRBL_ERR_37);

    return errorTranslationMap;
}


void GrblBoard::rtCmdPauseCycle(){
    m_serialPort->write(CMD_PAUSE_STRING);
    QTimer::singleShot(RT_CMD_PROCESS_TIME_MS,this,&GrblBoard::rtCmdRequestStatus);
}

void GrblBoard::rtCmdResumeCycle(){
    m_serialPort->write(CMD_RESUME_STRING);
    QTimer::singleShot(RT_CMD_PROCESS_TIME_MS,this,&GrblBoard::rtCmdRequestStatus);
}

void GrblBoard::rtCmdRequestStatus(){
    m_serialPort->write(CMD_STATUS_REQ_STRING);
}

void GrblBoard::rtCmdSoftReset(){
    m_serialPort->write(CMD_SOFT_RESET_STRING);
}

void GrblBoard::rtCmdSafetyDoor(){
    m_serialPort->write(CMD_SAFETY_DOOR);
    QTimer::singleShot(RT_CMD_PROCESS_TIME_MS,this,&GrblBoard::rtCmdRequestStatus);
}


