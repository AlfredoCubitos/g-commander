#include "visualizerwidget.h"

#include "QRegularExpression"
#include <QtMath>

#define ZNEAR               5.0f
#define ZFAR                10000.0f
#define VIEW_DIST_MIN       20.0f
#define VIEW_DIST_MAX       9000.0f
#define VIEW_DIST_DEFAULT   250.0f


VisualizerWidget::VisualizerWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    m_drillPrimitive(nullptr){
}

void VisualizerWidget::cleanModel(){
    makeCurrent();      //Make sure context is current before deleting buffers
    qDeleteAll(m_pathSegmentsVector);
    m_pathSegmentsVector.clear();
    doneCurrent();

    resetView();
}

void VisualizerWidget::appendPrimitive(int line, QVector<QVector3D> path, bool isWork){
    //If no line number is invalid, give up
    if(line >= 0){
        makeCurrent();

        VisualizerPrimitive* segment = new VisualizerPrimitive(&path, isWork ? VisualizerPrimitive::VPT_GCODE_WORK : VisualizerPrimitive::VPT_GCODE_MOVE);

        //Fill empty positions in segmentsVector with nullptr value
        int vectorSize = m_pathSegmentsVector.size();
        if(vectorSize<line){
            m_pathSegmentsVector.insert(vectorSize,line-vectorSize,nullptr);
        }

        m_pathSegmentsVector.insert(line,segment);

        doneCurrent();
    }
}

void VisualizerWidget::setPathSegmentStatus(int line, VisualizerPrimitive::Status status){
    if(line > 0 && line < m_pathSegmentsVector.size()){
        VisualizerPrimitive* segment = m_pathSegmentsVector.at(line);
        if(segment != nullptr){
            segment->setStatus(status);
        }
    }
}

void VisualizerWidget::onInstructionSent(GrblInstruction instruction){
    setPathSegmentStatus(instruction.getLineNumber(),VisualizerPrimitive::VPS_RUNNING);
    update();
}

void VisualizerWidget::onInstructionError(GrblInstruction instruction){
    setPathSegmentStatus(instruction.getLineNumber(),VisualizerPrimitive::VPS_ERROR);
    update();
}

void VisualizerWidget::onInstructionOk(GrblInstruction instruction){
    setPathSegmentStatus(instruction.getLineNumber(),VisualizerPrimitive::VPS_OK);
    update();
}

void VisualizerWidget::rewindModel(){
    for(int i = 0 ; i < m_pathSegmentsVector.size() ; i++){
        setPathSegmentStatus(i,VisualizerPrimitive::VPS_IDLE);
    }
    update();
}


void VisualizerWidget::onGrblStatusUpdated(GrblStatus * const status){
    if(status->containsWorkPosition()){
        m_drillPosition = status->getWorkPositionInMm();
    }
    update();
}


void VisualizerWidget::mouseDoubleClickEvent(QMouseEvent *e){
    Q_UNUSED(e);
    resetView();
}

void VisualizerWidget::mousePressEvent(QMouseEvent *e){
    m_mousePrevPosition = QVector2D(e->localPos());
}



void VisualizerWidget::mouseMoveEvent(QMouseEvent *e)
{
    QVector2D diff = QVector2D(e->localPos()) - m_mousePrevPosition;
    m_mousePrevPosition = QVector2D(e->localPos());

    if(e->buttons() == Qt::LeftButton ){
        // Rotation axis is perpendicular to the mouse position difference
        // vector
        QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

        // Calculate new rotation axis as weighted sum
        QVector3D rotationAxis = n.normalized();

        m_rotation = QQuaternion::fromAxisAndAngle(rotationAxis, diff.length()) * m_rotation;

    }

    if(e->buttons() == Qt::RightButton ){
        float ratio =10.0f;
        m_translation[0] += diff.x() / ratio;
        m_translation[1] -= diff.y() / ratio;
    }

    // Request an update
    update();
}

void VisualizerWidget::wheelEvent(QWheelEvent *e){
    float distanceModifier = -(e->angleDelta().y());

    //Always multiply / divide distance
    float newZvalue = m_translation.z() *qPow(1.001,distanceModifier);

    //Ensure we never goes outside [VIEW_DIST_MIN;VIEW_DIST_MAX]
    m_translation.setZ( qBound(-VIEW_DIST_MAX, newZvalue, -VIEW_DIST_MIN));

    update();
}


void VisualizerWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);

    //Allows for some transparency rendering, quite poor, but does the job without sorting
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    initShaders();

    //Create drill representation
    m_drillPrimitive = VisualizerPrimitive::createDrillPrimitive(10.0f,5.0f);

    //Create axis representation
    m_axisPrimitives[0] = VisualizerPrimitive::createAxisPrimitive(VisualizerPrimitive::VPT_AXIS_X);
    m_axisPrimitives[1] = VisualizerPrimitive::createAxisPrimitive(VisualizerPrimitive::VPT_AXIS_Y);
    m_axisPrimitives[2] = VisualizerPrimitive::createAxisPrimitive(VisualizerPrimitive::VPT_AXIS_Z);

    resetView();
}

void VisualizerWidget::initShaders()
{
    m_program = new QOpenGLShaderProgram(this);

    // Compile vertex shader
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader"))
        close();

    // Compile fragment shader
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader"))
        close();

    // Link shader pipeline
    if (!m_program->link())
        close();

    // Bind shader pipeline for use
    if (!m_program->bind())
        close();
}

void VisualizerWidget::resetView()
{
    m_translation = QVector3D(0,0,-VIEW_DIST_DEFAULT);
    m_rotation = QQuaternion();

    update();
}




void VisualizerWidget::resizeGL(int w, int h){
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 0.0, far plane to 100.0, field of view 45 degrees
    const qreal zNear = ZNEAR, zFar = ZFAR, fov = 45.0;

    // Reset projection
    m_projection.setToIdentity();

    // Set perspective projection
    m_projection.perspective(fov, aspect, zNear, zFar);

}

void VisualizerWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    QMatrix4x4 viewMatrix;

    viewMatrix.translate(m_translation);
    viewMatrix.rotate(m_rotation);

    //Draw axis model
    // Set modelview-projection matrix
    m_program->setUniformValue("mvp_matrix", m_projection * viewMatrix);
    for(int i = 0 ; i < 3 ; i++){
        m_axisPrimitives[i]->drawGeometry(m_program);
    }


    //Draw gcode model
    // Set modelview-projection matrix
    m_program->setUniformValue("mvp_matrix", m_projection * viewMatrix);

    // Draw cube geometry
    foreach (VisualizerPrimitive* segment, m_pathSegmentsVector) {
        if(segment !=nullptr){
            segment->drawGeometry(m_program);
        }
    }

    //Draw drill model
    QMatrix4x4 drillTranslationMatrix;
    drillTranslationMatrix.translate(m_drillPosition);

    // Set modelview-projection matrix
    m_program->setUniformValue("mvp_matrix", m_projection * viewMatrix * drillTranslationMatrix);

    m_drillPrimitive->drawGeometry(m_program);


}



VisualizerWidget::~VisualizerWidget()
{
    cleanModel();

    delete m_drillPrimitive;

    delete m_axisPrimitives[0];
    delete m_axisPrimitives[1];
    delete m_axisPrimitives[2];
}

