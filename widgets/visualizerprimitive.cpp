#include "visualizerprimitive.h"

QMap<int,PrimitiveDrawSettings> VisualizerPrimitive::s_drawSettingsMap = VisualizerPrimitive::initializeDrawSettingsMap();


QMap<int, PrimitiveDrawSettings> VisualizerPrimitive::initializeDrawSettingsMap(){
    QMap<int,PrimitiveDrawSettings> drawSettingsMap;

    drawSettingsMap.insert(VPT_GCODE_WORK*4 + VPS_IDLE,     {QVector4D(0.5f,0.5f,0.5f,1.0f), 1.0f});
    drawSettingsMap.insert(VPT_GCODE_WORK*4 + VPS_RUNNING,  {QVector4D(0.5f,0.5f,1.0f,1.0f), 1.0f});
    drawSettingsMap.insert(VPT_GCODE_WORK*4 + VPS_OK,       {QVector4D(0.0f,1.0f,0.0f,1.0f), 1.0f});
    drawSettingsMap.insert(VPT_GCODE_WORK*4 + VPS_ERROR,    {QVector4D(1.0f,0.0f,0.0f,1.0f), 1.0f});
    drawSettingsMap.insert(VPT_GCODE_MOVE*4 + VPS_IDLE,     {QVector4D(0.2f,0.2f,0.2f,0.5f), 1.0f});
    drawSettingsMap.insert(VPT_GCODE_MOVE*4 + VPS_RUNNING,  {QVector4D(0.2f,0.2f,0.5f,0.5f), 1.0f});
    drawSettingsMap.insert(VPT_GCODE_MOVE*4 + VPS_OK,       {QVector4D(0.0f,0.5f,0.0f,0.5f), 1.0f});
    drawSettingsMap.insert(VPT_GCODE_MOVE*4 + VPS_ERROR,    {QVector4D(0.5f,0.0f,0.0f,0.5f), 1.0f});
    drawSettingsMap.insert(VPT_DRILL*4,                     {QVector4D(1.0f,0.5f,0.0f,1.0f), 1.0f});
    drawSettingsMap.insert(VPT_AXIS_X*4,                    {QVector4D(1.0f,0.0f,0.0f,1.0f), 1.0f});
    drawSettingsMap.insert(VPT_AXIS_Y*4,                    {QVector4D(0.0f,1.0f,0.0f,1.0f), 1.0f});
    drawSettingsMap.insert(VPT_AXIS_Z*4,                    {QVector4D(0.0f,0.0f,1.0f,1.0f), 1.0f});

    return drawSettingsMap;
}



VisualizerPrimitive::VisualizerPrimitive(QVector<QVector3D> *vertices, Type type)
{    
    initializeOpenGLFunctions();

    //Create VBO
    m_arrayBuf.create();

    if(vertices == nullptr || vertices->isEmpty()){
        m_verticesCount = 0;
        return;
    }

    m_arrayBuf.bind();
    m_arrayBuf.allocate(vertices->data(),vertices->size() * sizeof(QVector3D));
    m_arrayBuf.release();

    m_verticesCount = vertices->size();

    m_type = type;
    m_status = VPS_IDLE;
}

VisualizerPrimitive::~VisualizerPrimitive(){
    m_arrayBuf.destroy();
}

void VisualizerPrimitive::setStatus(VisualizerPrimitive::Status status){
    if(m_type == VPT_GCODE_MOVE || m_type == VPT_GCODE_WORK)  {
        m_status = status;
    }
}

VisualizerPrimitive::Type VisualizerPrimitive::getType(){
    return m_type;
}

void VisualizerPrimitive::drawGeometry(QOpenGLShaderProgram *program){
    //No need to draw "nothing"
    if(m_verticesCount > 0){

        //Use m_arraybuf as vertices buffer
        m_arrayBuf.bind();
        int vertexLocation = program->attributeLocation("a_position");
        program->enableAttributeArray(vertexLocation);
        program->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
        m_arrayBuf.release();

        //Set the rendering color according to segment status
        PrimitiveDrawSettings defaultSetting = {QVector4D(1.0f,1.0f,1.0f,1.0f),1.0f};
        PrimitiveDrawSettings drawSettings = s_drawSettingsMap.value(m_type*4+m_status,defaultSetting);
        program->setUniformValue("color",drawSettings.color);

        glLineWidth(drawSettings.lineWidth);

        //Render segment
        glDrawArrays(GL_LINE_STRIP, 0, m_verticesCount);
    }

}

void VisualizerPrimitive::setDrawSettings(VisualizerPrimitive::Type status, PrimitiveDrawSettings drawSettings){
    s_drawSettingsMap.insert(status,drawSettings);
}

PrimitiveDrawSettings VisualizerPrimitive::getDrawSettings(VisualizerPrimitive::Type status){
    return s_drawSettingsMap.value(status);
}

VisualizerPrimitive *VisualizerPrimitive::createDrillPrimitive(float height, float width){
    QVector <QVector3D> vector(7);
    vector[0] = QVector3D(0.0f,0.0f,0.0f);
    vector[1] = QVector3D(-width,0.0f,height);
    vector[2] = QVector3D(width,0.0f,height);
    vector[3] = QVector3D(0.0f,0.0f,0.0f);
    vector[4] = QVector3D(0.0f,-width,height);
    vector[5] = QVector3D(0.0f,width,height);
    vector[6] = QVector3D(0.0f,0.0f,0.0f);

    VisualizerPrimitive *drillPrimitive = new VisualizerPrimitive(&vector,VPT_DRILL);

    return drillPrimitive;
}

VisualizerPrimitive *VisualizerPrimitive::createAxisPrimitive(Type axisType){
    QVector <QVector3D> vector(2);

    switch (axisType) {
    case VPT_AXIS_X:
        vector[1] = QVector3D(1.0f,0.0f,0.0f);
        break;
    case VPT_AXIS_Y:
        vector[1] = QVector3D(0.0f,1.0f,0.0f);
        break;
    case VPT_AXIS_Z:
        vector[1] = QVector3D(0.0f,0.0f,1.0f);
        break;
    default:
        break;
    }

    VisualizerPrimitive *axisPrimitive = new VisualizerPrimitive(&vector,axisType);

    return axisPrimitive;
}


