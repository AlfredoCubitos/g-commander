#include "grbldefinitions.h"
#include "grblstatus.h"

#include <QString>
#include <QStringList>
#include <QMap>


const QRegularExpression GrblStatus::s_mposExpression = QRegularExpression(STATUS_MACHINE_POS"(?<xvalue>-?[\\d]+.[\\d]+),(?<yvalue>-?[\\d]+.[\\d]+),(?<zvalue>-?[\\d]+.[\\d]+)");
const QRegularExpression GrblStatus::s_wposExpression = QRegularExpression(STATUS_WORK_POS"(?<xvalue>-?[\\d]+.[\\d]+),(?<yvalue>-?[\\d]+.[\\d]+),(?<zvalue>-?[\\d]+.[\\d]+)");
const QRegularExpression GrblStatus::s_motionsExpression = QRegularExpression(STATUS_MOTION_NUM"(?<count>[\\d]+)");
const QRegularExpression GrblStatus::s_charactersExpression = QRegularExpression(STATUS_CHARACTER_NUM"(?<count>[\\d]+)");

const QMap<QString, GrblStatus::states> GrblStatus::s_stateDecodingMap = initStateStringMap();


const QMap<QString, GrblStatus::states> GrblStatus::initStateStringMap(void){
    QMap<QString, GrblStatus::states> map;
    map.insert(STATE_IDLE_STRING, GrblStatus::state_idle);
    map.insert(STATE_RUN_STRING, GrblStatus::state_run);
    map.insert(STATE_HOLD_STRING, GrblStatus::state_hold);
    map.insert(STATE_DOOR_STRING, GrblStatus::state_door);
    map.insert(STATE_HOME_STRING, GrblStatus::state_home);
    map.insert(STATE_ALARM_STRING, GrblStatus::state_alarm);
    map.insert(STATE_CHECK_STRING, GrblStatus::state_check);
    return map;
}

GrblStatus::GrblStatus(QString statusLine, bool isUnitInches, GrblStatus *previousStatus):GrblStatus(){

    //Check for previous status for previous state
    if(previousStatus != nullptr){
        m_prevState = previousStatus->getState();
    }

    statusLine.remove(RESPONSE_STATUS_START);
    statusLine.remove(RESPONSE_STATUS_END);

    m_isUnitInches = isUnitInches;

    //First ( and mandatory ) info is ALWAYS the state
    foreach(QString stateString, s_stateDecodingMap.keys()){
        if(statusLine.startsWith(stateString)){
            m_currState = s_stateDecodingMap.value(stateString);
            break;
        }
    }

    //Look for machine position
    QRegularExpressionMatch mposMatch = s_mposExpression.match(statusLine);
    if(mposMatch.hasMatch()){
        //Now extract infos
        m_machinePosition.setX(mposMatch.captured("xvalue").toFloat());
        m_machinePosition.setY(mposMatch.captured("yvalue").toFloat());
        m_machinePosition.setZ(mposMatch.captured("zvalue").toFloat());
        m_hasMachinePosition = true;
    }

    //Look for work position
    QRegularExpressionMatch wposMatch = s_wposExpression.match(statusLine);
    if(wposMatch.hasMatch()){
        //Now extract infos
        m_workPosition.setX(wposMatch.captured("xvalue").toFloat());
        m_workPosition.setY(wposMatch.captured("yvalue").toFloat());
        m_workPosition.setZ(wposMatch.captured("zvalue").toFloat());
        m_hasWorkPosition = true;
    }

    //Look for planned motions count
    QRegularExpressionMatch motionsMatch = s_motionsExpression.match(statusLine);
    if(motionsMatch.hasMatch()){
        //Now extract infos
        m_motionsPlanned = motionsMatch.captured("count").toInt();
        m_hasMotionsPlanned = true;
    }

    //Look for characters in RX buffer count
    QRegularExpressionMatch charactersMatch = s_charactersExpression.match(statusLine);
    if(charactersMatch.hasMatch()){
        //Now extract infos
        m_charactersQueued = charactersMatch.captured("count").toInt();
        m_hasCharactersQueued = true;
    }
}


GrblStatus::GrblStatus(bool isConnected):
    m_currState(isConnected ? state_unknown : state_offline),
    m_hasPrevState(false),
    m_prevState(state_unknown),
    m_isUnitInches(false),
    m_hasMachinePosition(false),
    m_machinePosition(),
    m_hasWorkPosition(false),
    m_workPosition(),
    m_hasMotionsPlanned(false),
    m_motionsPlanned(0),
    m_hasCharactersQueued(false),
    m_charactersQueued(0)
{

}



QString GrblStatus::getStateString() const {
    switch(m_currState){
    case state_alarm:
        return QString("Alarm");
    case state_check:
        return QString("Simulation");
    case state_door:
        return QString("Door");
    case state_hold:
        return QString("Feed hold");
    case state_home:
        return QString("Homing");
    case state_idle:
        return QString("Ready");
    case state_offline:
        return QString("Offline");
    case state_run:
        return QString("Running");
    default:
        return QString("Unknown");
    }
}
