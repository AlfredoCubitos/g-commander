#ifndef GRBLSTATUS_H
#define GRBLSTATUS_H

#include <QVector3D>
#include <QRegularExpression>
#include"grbldefinitions.h"

class GrblStatus
{
public:

    enum states{state_idle, state_run, state_hold, state_door, state_home, state_alarm, state_check, state_unknown, state_offline};

    explicit GrblStatus(bool isConnected = true);
    explicit GrblStatus(QString statusLine, bool isUnitInches, GrblStatus* previousStatus = nullptr);

    states getState() const {return m_currState;}

    bool containsPreviousState() const {return m_hasPrevState;}
    states getPreviousState() const {return m_prevState;}

    bool isStateOnline() const {return m_currState != state_offline;}
    bool isStateKnown() const {return  isStateOnline() && m_currState != state_unknown;}
    bool isStateAlarm() const {return m_currState == state_alarm;}
    bool isStateNominal() const { return isStateKnown() && (!isStateAlarm());}
    bool isStateCheck() const {return m_currState == state_check;}

    bool isUnitInches() const {return m_isUnitInches;}

    bool containsMachinePosition() const {return m_hasMachinePosition;}
    QVector3D getMachinePositionInGrblUnits() const {return m_machinePosition;}
    QVector3D getMachinePositionInMm() const {return (m_isUnitInches) ? m_machinePosition * MM_PER_INCH : m_machinePosition;}

    bool containsWorkPosition() const {return m_hasWorkPosition;}
    QVector3D getWorkPositionInGrblUnits() const {return m_workPosition;}
    QVector3D getWorkPositionInMm() const {return (m_isUnitInches) ? m_workPosition * MM_PER_INCH : m_workPosition;}

    bool containsMotionsPlanned() const {return m_hasMotionsPlanned;}
    int getMotionsPlanned() const {return m_motionsPlanned;}

    bool containsCharactersQueued() const {return m_hasCharactersQueued;}
    int getCharactersQueued() const {return m_charactersQueued;}

    QString getStateString() const;


private:
    states m_currState;

    bool m_hasPrevState;
    states m_prevState;

    bool m_isUnitInches;

    bool m_hasMachinePosition;
    QVector3D m_machinePosition;

    bool m_hasWorkPosition;
    QVector3D m_workPosition;

    bool m_hasMotionsPlanned;
    int m_motionsPlanned;

    bool m_hasCharactersQueued;
    int m_charactersQueued;

    //Convert from string
    static const  QMap<QString, states> s_stateDecodingMap;
    static const QMap<QString, states> initStateStringMap(void);

    static const QRegularExpression s_mposExpression;
    static const QRegularExpression s_wposExpression;
    static const QRegularExpression s_motionsExpression;
    static const QRegularExpression s_charactersExpression;
};

#endif // GRBLSTATUS_H
