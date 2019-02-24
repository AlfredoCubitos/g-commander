#ifndef GRBLBOARD_H
#define GRBLBOARD_H

#include <QObject>
#include <QSerialPort>
#include <QVector3D>
#include <QTimer>

#include "grblstatus.h"
#include "grblconfiguration.h"
#include "grblinstruction.h"

class GrblBoard : public QObject
{
    Q_OBJECT
public:

    explicit GrblBoard(QObject *parent = 0);

    void setSerialSettings(QString portName, qint32 baudRate);

    void setStatusRequestInterval(int interval);

    const GrblStatus *getLastStatus(void) const;
    QMap<int,GrblConfiguration> *getParametersMap(void);



signals:

    //Board responded to periodic status request
    void statusUpdated(GrblStatus* const status);

    //Board sent hello message after a reset
    void boardStartup(QString versionString, QList<GrblInstruction> startupRelatedInstructions);

    //Instruction has been processed, board sent response. After one of these we can iterate to the next command
    void ok(GrblInstruction instruction);
    void error(GrblInstruction instruction,QString errorMessage);        //Remember to freeze on this  if processsing a file

    //Informative messages, does not indicate that instruction was completed or anything. Do not base sequencing on theses !
    void alarm(GrblInstruction instruction, QString alarmMessage);
    void feedback(GrblInstruction instruction, QString feedbackMessage);
    void text(GrblInstruction instruction, QString textMessage);

    //Parameters have been updated
    void parametersMapUpdated(QMap<int,GrblConfiguration> *updatedMap);

    //Emitted when instruction is placed in char buffer
    void instructionSent(GrblInstruction instruction);

public slots:

    //Open / close serial link
    void toggleSerial(void);

    //Board real time commands
    void rtCmdPauseCycle(void);
    void rtCmdResumeCycle(void);
    void rtCmdRequestStatus(void);
    void rtCmdSoftReset(void);
    void rtCmdSafetyDoor(void);

    //Send an instruction
    void sendInstruction(GrblInstruction instruction);

private slots:


    void onSerialDataAvailable(void);

    void sendStartupInstructions(void);


private:
    int getAvailableSpaceInCharBuffer();
    bool isBlockingInstructionInBuffer(void);

    void addErrorTranslation(QString* errorString);


    QList<GrblInstruction> m_boardCharBuffer;
    QByteArray m_bufferFromBoard;
    QSerialPort *m_serialPort;
    QTimer* m_statusTimer;
    GrblStatus m_status;

    QMap<int,GrblConfiguration> m_parametersMapComplete;
    QMap<int,GrblConfiguration> m_parametersMapBeingFilled;

    QList<GrblInstruction> m_startupInstructionList;

    static const QMap<int,QString> errorTranslationMap;
    static QMap<int,QString> generateErrorTranslationMap();
};

#endif // GRBLBOARD_H
