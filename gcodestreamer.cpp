#include "gcodestreamer.h"
#include "grbldefinitions.h"

#include <QFile>
#include <QFileInfo>

#define MAX_LINE_LENGTH         256     //Defined by gcode standard
#define DEFAULT_FIFO_DEPTH      1000

const char *GCodeStreamer::s_gcodeCommentsDelimiters[] = {GCODE_COMMENTS_DELIM};

GCodeStreamer::GCodeStreamer(QObject *parent) :
    QObject(parent),
    m_run(false)
{
    clear();
}

GCodeStreamer::states GCodeStreamer::getState(void){
    if(!m_usefulLinesVector.isEmpty()){
        if(m_run){
            return state_running;
        }
        else{
            return state_ready;
        }
    }
    else{
        return state_clear;
    }
}

void GCodeStreamer::loadFile(const QString &path){

    clear();

    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
    {
        file.reset();

        //Store file in linevector
        while(!file.atEnd()){

            //Get line
            QByteArray gcodeLine = file.readLine(MAX_LINE_LENGTH);
            cleanupLine(&gcodeLine);

            m_lineCount++;

            //No need to process a useless line
            if(gcodeLine.isEmpty()){
                continue;
            }

            //Add it to line vector
            GrblInstruction instruction(gcodeLine,m_lineCount);
            m_usefulLinesVector.append(instruction);

            emit instructionLoaded(instruction);
        }

        emit fileLoaded(QFileInfo(file).baseName());
    }

    emit lineCountUpdated(m_lineCount);
    emit stateChanged(getState());

    //Make the file ready to start from the beginning
    rewind();
}

void GCodeStreamer::cleanupLine(QByteArray* code){
    int commentDelimiterCount = sizeof(s_gcodeCommentsDelimiters)/sizeof(s_gcodeCommentsDelimiters[0]);
    for(int i = 0 ; i < commentDelimiterCount ; i++){
        int delimiterPosition = code->indexOf(s_gcodeCommentsDelimiters[i]); //Return start of comment or -1
        if(delimiterPosition >= 0){
            code->truncate(delimiterPosition);     //Resize to start of comment. If -1, do nothing
        }
    }

    //remove any start / and whitespace and special character
    (*code) = code->trimmed();
}



void GCodeStreamer::clear(){
    rewind();

    m_lineCount = 0;
    m_usefulLinesVector.clear();

    emit lineCountUpdated(0);
    emit stateChanged(state_clear);
    emit cleared();
}


void GCodeStreamer::goToLine(int line){
    if(m_usefulLinesVector.isEmpty()){
        return;
    }

    int firstUsefulLineNumber = m_usefulLinesVector.first().getLineNumber();
    int lastUsefulLineNumber = m_usefulLinesVector.last().getLineNumber();
    int lastLineIndex = m_usefulLinesVector.size()-1;

    //First line
    if(line <= firstUsefulLineNumber){
        m_lineToSendIndex = 0;
        m_lastLineParsedByGrbl = 0;
    }
    else {
        //Last Line
        if(line >= lastUsefulLineNumber){
            m_lineToSendIndex = lastLineIndex;
        }

        //Any other line, we need to iterate to find the correct line
        else{
            //Start from 'line' index, which is always greater than the correct index (due to useless lines not added to linevector)
            //then iterate until we find the perfect line number, or a smaller one
            m_lineToSendIndex = qMin(line,lastLineIndex-1);
            while(m_lineToSendIndex > 0 && getCurrentLineNumber() >= line){
                m_lineToSendIndex--;
            }

            //If we did not find the exact index (due to seeked line being useless, so not part of vector),
            // Go to the next line
            if(line > getCurrentLineNumber()){
                m_lineToSendIndex++;
            }
        }

        //Set previous instruction as parsed
        m_lastLineParsedByGrbl = m_usefulLinesVector.at(m_lineToSendIndex-1).getLineNumber();
    }

    //Next instruction to be processed is the first on in buffer
    emit currentLineUpdated(getCurrentLineNumber());
}


void GCodeStreamer::go(void){
    if(!m_usefulLinesVector.isEmpty()){
        m_run = true;
        tryToSendNextInstruction();
        emit stateChanged(state_running);
    }
}

void GCodeStreamer::step(){
    if(!m_usefulLinesVector.isEmpty()){
        m_run = false;
        tryToSendNextInstruction();
        emit stateChanged(state_ready);
    }
}

void GCodeStreamer::stop(){
    m_run = false;
    emit stateChanged(state_ready);
}


void GCodeStreamer::onGrblStatusUpdated(GrblStatus* const status){
    int instructionsInPlanningBuffer = status->containsMotionsPlanned() ? status->getMotionsPlanned() : 0;

    int executedLine = m_lastLineParsedByGrbl - instructionsInPlanningBuffer;
    emit currentLineUpdated(executedLine);

    //work should be complete when :
    //  - currently running
    //  - line count is not null
    //  - last line was executed
    //  - board is not in "run" state anymore
    if(m_run && !m_usefulLinesVector.isEmpty() && executedLine == m_usefulLinesVector.last().getLineNumber() && status->getState() != GrblStatus::state_run){
        m_run = false;
        emit workCompleted();
        emit stateChanged(state_ready);
    }
}

void GCodeStreamer::onInstructionParsedByGrbl(const GrblInstruction &parsedInstruction){
    int parsedLineNumber = parsedInstruction.getLineNumber();
    if( parsedLineNumber >= 0){
        m_lastLineParsedByGrbl = parsedLineNumber;
    }

    if(m_run){
        tryToSendNextInstruction();
    }
}

void GCodeStreamer::onInstructionSentToGrbl(const GrblInstruction &acceptedInstruction){
    //If there is actually something to send
    if(m_usefulLinesVector.isEmpty()){
        return;
    }

    //If we already sent all useful lines, no need to continue
    if(m_lineToSendIndex >= m_usefulLinesVector.size()){
        return;
    }

    //This is not the instruction you're looking for
    if(acceptedInstruction != m_usefulLinesVector.at(m_lineToSendIndex)){
        return;
    }

    //Try to avoid an index out of range
    int lastUsefulLineIndex = m_usefulLinesVector.size()-1;
    if(m_lineToSendIndex > lastUsefulLineIndex){
        return;
    }

    //Then we cant safely increment
    m_lineToSendIndex++;

    //Schedule sending next instruction
    if(m_run){
        QTimer::singleShot(0, this, &GCodeStreamer::tryToSendNextInstruction);
    }
}


void GCodeStreamer::tryToSendNextInstruction(){
    if(m_lineToSendIndex < m_usefulLinesVector.size() ){
        m_usefulLinesVector[m_lineToSendIndex].regenerate();
        emit instructionToSend(m_usefulLinesVector.at(m_lineToSendIndex));
    }
}

int GCodeStreamer::getCurrentLineNumber(){
    int currentLineNumber = 0;
    if(!m_usefulLinesVector.isEmpty()){
        //Since m_currentLinuxIndex can go 1 unit after last line, clamp it
        int currentLineIndex = qMin(m_lineToSendIndex, m_usefulLinesVector.size()-1);

        currentLineNumber = m_usefulLinesVector.at(currentLineIndex).getLineNumber();
    }
    return (currentLineNumber);
}


