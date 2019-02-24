#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include <QObject>
#include <QVector3D>
#include <QMultiMap>
#include "grblinstruction.h"

class GCodeParser : public QObject
{
    Q_OBJECT
public:
    enum G0_NonModalActions{NON_MODAL_NO_ACTION = 0, NON_MODAL_DWELL, NON_MODAL_SET_COORDINATE_DATA, NON_MODAL_GO_HOME_0,
                            NON_MODAL_SET_HOME_0, NON_MODAL_GO_HOME_1, NON_MODAL_SET_HOME_1, NON_MODAL_ABSOLUTE_OVERRIDE,
                            NON_MODAL_SET_COORDINATE_OFFSET, NON_MODAL_RESET_COORDINATE_OFFSET};
    enum G1_MotionModes{MOTION_MODE_SEEK = 0, MOTION_MODE_LINEAR, MOTION_MODE_CW_ARC, MOTION_MODE_CCW_ARC,
                        MOTION_MODE_PROBE, MOTION_MODE_NONE};

    enum G2_PlaneSelect{PLANE_SELECT_XY = 0, PLANE_SELECT_ZX, PLANE_SELECT_YZ};

    enum G3_DistanceMode{DISTANCE_MODE_ABSOLUTE = 0, DISTANCE_MODE_INCREMENTAL};

    enum G6_UnitsMode{UNITS_MODE_MM = 0, UNITS_MODE_INCHES};

    explicit GCodeParser(QObject *parent = 0);

    uint32_t getMachineTime() const;

signals:

    void parsedPrimitive(int line, QVector<QVector3D> geometry, bool isWork);

public slots:

    void parseInstruction(GrblInstruction instruction);
    void reset();

private:

    void computeMovement(int line);
    QVector<QVector3D> buildLinePointsVector(QVector3D target);
    QVector<QVector3D> buildArcPointsVector(QVector3D target);
    QVector2D computeArcCenter(QVector3D target);

    void computeMachineTime(QVector<QVector3D> pointsVector);

    void processGValues();
    QVector3D processXYZValues();

    bool isMotionWork();

    const int *getAxisMap();


    G0_NonModalActions  m_g0NonModal;
    G1_MotionModes      m_g1Motion;
    G2_PlaneSelect      m_g2Plane;
    G3_DistanceMode     m_g3Distance;
    G6_UnitsMode        m_g6Units;

    uint32_t m_machineTime; // ms

    float m_machineSpeed; // mm/m

    int axes[3];

    QVector3D m_currentPos; //in mm
    bool m_isCurrentPosValid;

    QMultiMap<char,float> m_wordMap;

    static const int s_axisArray[3][3];
};

#endif // GCODEPARSER_H
