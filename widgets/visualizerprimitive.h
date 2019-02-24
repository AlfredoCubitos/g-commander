#ifndef VISUALIZERSEGMENT_H
#define VISUALIZERSEGMENT_H

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QVector4D>

struct PrimitiveDrawSettings{
    QVector4D color;
    float lineWidth;
};


class VisualizerPrimitive : protected QOpenGLFunctions
{
public:
    enum Status{VPS_IDLE = 0, VPS_RUNNING = 1, VPS_OK = 2, VPS_ERROR = 3};

    enum Type{
            //Useful movements (milling) primitive
            VPT_GCODE_WORK = 0,

            //Other movements primitive
            VPT_GCODE_MOVE,

            //Drill representation primitive
            VPT_DRILL,

            //Axis primitive
            VPT_AXIS_X, VPT_AXIS_Y, VPT_AXIS_Z
    };

    VisualizerPrimitive(QVector<QVector3D> *vertices = nullptr, Type type = VPT_GCODE_WORK);
    ~VisualizerPrimitive();

    void drawGeometry(QOpenGLShaderProgram *program);

    void setStatus(Status status);

    Type getType();

    static void setDrawSettings(Type status, PrimitiveDrawSettings drawSettings);
    static PrimitiveDrawSettings getDrawSettings(Type status);

    static VisualizerPrimitive *createDrillPrimitive(float height, float width);
    static VisualizerPrimitive *createAxisPrimitive(Type axisType);

private:

    Type m_type;
    Status m_status;

    QOpenGLBuffer m_arrayBuf;
    int m_verticesCount;

    static QMap<int,PrimitiveDrawSettings> s_drawSettingsMap;
    static QMap<int,PrimitiveDrawSettings> initializeDrawSettingsMap();


};

#endif // VISUALIZERSEGMENT_H
