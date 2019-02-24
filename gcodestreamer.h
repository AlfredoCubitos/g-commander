#ifndef GCODESTREAMER_H
#define GCODESTREAMER_H

#include <QObject>
#include <QVector>
#include <QByteArray>

#include "grblinstruction.h"
#include "grblboard.h"
#include "grblstatus.h"

class GCodeStreamer : public QObject
{
    Q_OBJECT
public:
    enum states {state_clear, state_ready, state_running};

    explicit GCodeStreamer(QObject *parent = 0);

    states getState(void);

signals:
    void fileLoaded(QString filename);
    void instructionLoaded(GrblInstruction instruction);

    void cleared();

    void stateChanged(states state);

    void lineCountUpdated(int line);
    void currentLineUpdated(int line);

    void workCompleted(void);

    void instructionToSend(GrblInstruction instruction);

public slots:

    //Start or pause instructions streaming
    void go(void);
    void step(void);
    void stop(void);

    //open / close file
    void loadFile(QString m_file);
    void clear();

    //Move inside file
    void rewind(){goToLine(0);}
    void goToLine(int line);

    //Grbl board related slots
    void onInstructionSentToGrbl(GrblInstruction acceptedInstruction);
    void onInstructionParsedByGrbl(GrblInstruction parsedInstruction);
    void onGrblStatusUpdated(GrblStatus* const status);


private:
    //void bufferizeIntructions(void);
    void cleanupLine(QByteArray* code);
    void tryToSendNextInstruction();
    int getCurrentLineNumber();

    int m_lineCount;
    int m_lineToSendIndex; //Position of read head in the file
    int m_lastLineParsedByGrbl;       //Last line accepted in planning buffer by grbl

    bool m_run;

    QVector<GrblInstruction> m_usefulLinesVector;

    static const char *s_gcodeCommentsDelimiters[]; //List of EEPROM related instructions, requiring use of simpler "blocking" protocol

};

#endif // GCODESTREAMER_H
