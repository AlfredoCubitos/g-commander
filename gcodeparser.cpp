#include "gcodeparser.h"

#include <QRegularExpression>
#include <QVector>
#include <QVector2D>
#include <qmath.h>
#include "grbldefinitions.h"

#define ANGLE_QUANTUM_DEG           5.0f

const int GCodeParser::s_axisArray[3][3] = {{0,1,2},{1,2,0},{2,0,1}};

GCodeParser::GCodeParser(QObject *parent) : QObject(parent)
{
    reset();
}

void GCodeParser::reset(){
    m_g0NonModal=NON_MODAL_NO_ACTION;
    m_g1Motion=MOTION_MODE_SEEK;
    m_g2Plane=PLANE_SELECT_XY;
    m_g3Distance=DISTANCE_MODE_ABSOLUTE;
    m_g6Units=UNITS_MODE_MM;

    m_machineTime = 0;
    m_machineSpeed = 0.0f;

    m_wordMap.clear();

    m_currentPos = QVector3D();
    m_isCurrentPosValid = false;
}


void GCodeParser::parseInstruction(GrblInstruction instruction){
    const QRegularExpression whitespaceExpression = QRegularExpression("\\s+");
    const QRegularExpression wordBeginExpression = QRegularExpression("[A-Z]");

    //Simplify gcode
    QString simplifiedGCode = instruction.getString().toUpper();
    simplifiedGCode.remove(whitespaceExpression);       //Removes whitespaces (incl line return characters)

    //Find first gcode word (should be at index 0, but better test it)
    int index = simplifiedGCode.indexOf(wordBeginExpression);

    //If not gcode word found, no need to try to parse this line
    if(index<0){
        return;
    }

    while(index < simplifiedGCode.size()){
        //Extract letter
        char letter = simplifiedGCode.at(index).toLatin1();

        //Now we will use index+1 as reference (the character after the beginning of the Gcode word)
        index++;

        //Locate next gcode word
        int nextIndex = simplifiedGCode.indexOf(wordBeginExpression,index);

        //If no next index found, next index is the end of the string
        if(nextIndex<0){
            nextIndex=simplifiedGCode.size();
        }

        //Extract value
        QStringRef valueStringRef = simplifiedGCode.midRef(index,nextIndex-index);
        bool success = true;
        float value = valueStringRef.toFloat(&success);

        //If parsing went successful, add this word to map
        if(success){
            m_wordMap.insert(letter,value);
        }

        //Get ready to locate next word
        index = nextIndex;
    }

    computeMovement(instruction.getLineNumber());

    m_wordMap.clear();
    m_g0NonModal = NON_MODAL_NO_ACTION;
}


void GCodeParser::computeMovement(int line){

    //Process 'F' words
    foreach(float value,m_wordMap.values('F')){
        //Convert it to mm
        if(m_g6Units == UNITS_MODE_INCHES){
            value *= MM_PER_INCH;
        }
        m_machineSpeed = value / 60.0f; //was mm/m, now into mm/s
    }

    //Process 'G' words
    processGValues();

    //Process 'X', 'Y' and 'Z' words
    bool wasCurrentPosValid = m_isCurrentPosValid;
    QVector3D targetPos = processXYZValues();

    //If we don't have a valid previous position, or if no movements to do, no need to go further
    if(!wasCurrentPosValid || targetPos == m_currentPos){
        return;
    }

    //Fill the point vector
    QVector<QVector3D> pointsVector;

    switch (m_g1Motion) {
    case MOTION_MODE_LINEAR:
    case MOTION_MODE_SEEK:
        pointsVector = buildLinePointsVector(targetPos);
        break;

    case MOTION_MODE_CCW_ARC:
    case MOTION_MODE_CW_ARC:
        pointsVector = buildArcPointsVector(targetPos);
        break;

    default:
        break;
    }

    //If vector is valid, emit signal
    if(pointsVector.size()>1){
        computeMachineTime(pointsVector);
        emit parsedPrimitive(line,pointsVector,isMotionWork());
    }

    m_currentPos=targetPos;
}

QVector<QVector3D> GCodeParser::buildLinePointsVector(QVector3D target){
    QVector<QVector3D> pointsVector;
    pointsVector.append(m_currentPos);
    pointsVector.append(target);

    return pointsVector;
}

QVector<QVector3D> GCodeParser::buildArcPointsVector(QVector3D target){
    const int *axis = getAxisMap();
    QVector<QVector3D> pointsVector;

    //Retrieve points in plane
    QVector2D center2DPos = computeArcCenter(target);
    QVector2D start2DPos( m_currentPos[axis[0]],
                        m_currentPos[axis[1]]);
    QVector2D end2DPos(   target[axis[0]],
                        target[axis[1]]);

    float radius = start2DPos.distanceToPoint(center2DPos);

    //Ensure center is equidistant from start and end point ( will catch errors in arcCenter computation )
    float radiusEnd = end2DPos.distanceToPoint(center2DPos);
    if(qAbs(radius - radiusEnd) >  ARC_ERROR){
        return pointsVector;
    }

    //Get angles
    float startAngle =  qAtan2( start2DPos.y()-center2DPos.y(),
                                start2DPos.x()-center2DPos.x());
    float endAngle =    qAtan2( end2DPos.y()-center2DPos.y(),
                                end2DPos.x()-center2DPos.x());
    float deltaAngle = (m_g1Motion == MOTION_MODE_CCW_ARC) ? endAngle-startAngle : startAngle-endAngle;

    //Get drill heights
    float startHeight = m_currentPos[axis[2]];
    float endHeight = target[axis[2]];
    float deltaHeight =  endHeight - startHeight;

    //Ensure angles are correct, and take number of turn into account
    float revolutionCount = qAbs(m_wordMap.value('P',0.0f));
    if(deltaAngle < 0){
        revolutionCount += 1;
    }
    deltaAngle += 2.0f * M_PI* revolutionCount;


    //Start building points
    pointsVector.append(m_currentPos);

    for(float angle = 0.0f ; angle < deltaAngle ; angle += ANGLE_QUANTUM_DEG * M_PI /180.0f){
        float currentAngle = (m_g1Motion == MOTION_MODE_CCW_ARC) ? startAngle + angle : startAngle - angle;

        QVector2D point2D = center2DPos;
        point2D[0] += radius * qCos(currentAngle);
        point2D[1] += radius * qSin(currentAngle);

        float height = startHeight + deltaHeight * angle / deltaAngle;

        QVector3D point3D;
        point3D[axis[0]]=point2D.x();
        point3D[axis[1]]=point2D.y();
        point3D[axis[2]]= height;

        pointsVector.append(point3D);
    }

    pointsVector.append(target);

    return pointsVector;
}

QVector2D GCodeParser::computeArcCenter(QVector3D target){
    const int *axis = getAxisMap();

    //Initialize center position at current position
    QVector2D arcCenter;
    arcCenter[0] = m_currentPos[axis[0]];
    arcCenter[1] = m_currentPos[axis[1]];


    //try with radius definition
    if(m_wordMap.contains('R')){
        float r = m_wordMap.value('R');

        if(m_g6Units == UNITS_MODE_INCHES){
            r *= MM_PER_INCH;
        }

        //From GRBL code :
        //d -> sqrt(x^2 + y^2)
        //h -> sqrt(4 * r^2 - x^2 - y^2)/2
        //i -> (x - (y * h) / d) / 2
        //j -> (y + (x * h) / d) / 2

        //Actual computation
        float x = m_currentPos[axis[0]] - target[axis[0]];
        float y = m_currentPos[axis[1]] - target[axis[1]];

        float d = qSqrt((x*x) - (y*y));

        float h = qSqrt((4.0f*r*r) - (x*x) - (y*y));

        //Error
        if(h < 0) {
            return arcCenter;
        }

        if(m_g1Motion == MOTION_MODE_CW_ARC){
            h = -h;     //This cheat is from grbl code
        }

        float i = (x - (y * h) / d) / 2;
        float j = (y + (x * h) / d) / 2;

        arcCenter[axis[0]] +=i;
        arcCenter[axis[1]] +=j;
    }


    //Try with center offset definition
    else{
        const char axisLetter[] = {'I','J','K'};
            for(int i = 0 ; i < 2 ; i++){
                float value = m_wordMap.value(axisLetter[axis[i]],0.0f);

                if(m_g6Units == UNITS_MODE_INCHES){
                    value *= MM_PER_INCH;
                }

                arcCenter[i] += value;
            }
    }

    return arcCenter;
}

void GCodeParser::computeMachineTime(QVector<QVector3D> pointsVector)
{
    QVector3D lastVector;
    QVector3D currentVector;

    QVector<QVector3D>::const_iterator it = pointsVector.constBegin();

    if(it != pointsVector.constEnd()) {
        lastVector = *it;
        it++;
        while(it != pointsVector.constEnd()) {
            currentVector = *it;
            float dist = lastVector.distanceToPoint(currentVector);
            m_machineTime += (dist / m_machineSpeed) * 1000.0f; //into ms
            lastVector = *it;
            it++;
        }
    }

}


void GCodeParser::processGValues(){
    foreach(float value,m_wordMap.values('G')){

        int mantissa = qAbs(value);
        int decimal = qRound((value-mantissa)*10.0f);

        switch(mantissa){
        //G0_NonModalActions
        case 64:
            m_g0NonModal=NON_MODAL_DWELL;
            break;
        case 10:
            m_g0NonModal=NON_MODAL_SET_COORDINATE_DATA;
            break;
        case 28:
            switch(decimal){
            case 0:
                m_g0NonModal=NON_MODAL_GO_HOME_0;
                break;
            case 1:
                m_g0NonModal=NON_MODAL_SET_HOME_0;
                break;
            default:
                break;
            }
            break;
        case 30:
            switch(decimal){
            case 0:
                m_g0NonModal=NON_MODAL_GO_HOME_1;
                break;
            case 1:
                m_g0NonModal=NON_MODAL_SET_HOME_1;
                break;
            default:
                break;
            }
            break;
        case 53:
            m_g0NonModal=NON_MODAL_ABSOLUTE_OVERRIDE;
            break;
        case 92:
            switch(decimal){
            case 0:
                m_g0NonModal=NON_MODAL_SET_COORDINATE_OFFSET;
                break;
            case 1:
                m_g0NonModal=NON_MODAL_RESET_COORDINATE_OFFSET;
                break;
            default:
                break;
            }
            break;

            //G1_MotionModes
        case 0:
            m_g1Motion=MOTION_MODE_SEEK;
            break;
        case 1:
            m_g1Motion=MOTION_MODE_LINEAR;
            break;
        case 2:
            m_g1Motion=MOTION_MODE_CW_ARC;
            break;
        case 3:
            m_g1Motion=MOTION_MODE_CCW_ARC;
            break;
        case 38:
            m_g1Motion=MOTION_MODE_PROBE;
            break;
        case 80:
            m_g1Motion=MOTION_MODE_NONE;

            //G2_PlaneSelect
        case 17:
            m_g2Plane=PLANE_SELECT_XY;
            break;
        case 18:
            m_g2Plane=PLANE_SELECT_ZX;
            break;
        case 19:
            m_g2Plane=PLANE_SELECT_YZ;
            break;

            //G3_DistanceMode
        case 90:
            m_g3Distance=DISTANCE_MODE_ABSOLUTE;
            break;
        case 91:
            m_g3Distance=DISTANCE_MODE_INCREMENTAL;
            break;

            //G6_UnitsMode
        case 20:
            m_g6Units=UNITS_MODE_INCHES;
            break;
        case 21:
            m_g6Units=UNITS_MODE_MM;
            break;

        default:
            break;
        }
    }
}

QVector3D GCodeParser::processXYZValues(){
    const char axisLetter[] = {'X','Y','Z'};

    bool wasCurrentPosValid = m_isCurrentPosValid;

    //Start from current position
    QVector3D targetPosition = m_currentPos;

    //For each axis
    for(quint8 i = 0 ; i < 3 ; i++){
        //Only if this axis letter is mentionned in wordMap
        if(!m_wordMap.contains(axisLetter[i])){
            continue;
        }

        //Get its value
        float axisValue = m_wordMap.value(axisLetter[i]);

        //Convert it to mm
        if(m_g6Units == UNITS_MODE_INCHES){
            axisValue *= MM_PER_INCH;
        }

        //Modify target position accordingly
        if(m_g3Distance == DISTANCE_MODE_ABSOLUTE || m_g0NonModal == NON_MODAL_ABSOLUTE_OVERRIDE){
            targetPosition[i]=axisValue;        //Absolute coordinate
        }
        else{
            targetPosition[i]+=axisValue;       //relative coordinate
        }

        m_isCurrentPosValid = true;
    }

    //if position just became valid
    if(m_isCurrentPosValid && !wasCurrentPosValid){
        m_currentPos = targetPosition;
    }

    return targetPosition;
}

bool GCodeParser::isMotionWork(){
    switch(m_g1Motion){
    case MOTION_MODE_LINEAR:
    case MOTION_MODE_CCW_ARC:
    case MOTION_MODE_CW_ARC:
        return true;
    default:
        return false;
    }
}

const int *GCodeParser::getAxisMap(){
    return &s_axisArray[m_g2Plane][0];
}

uint32_t GCodeParser::getMachineTime() const
{
    return m_machineTime;
}





