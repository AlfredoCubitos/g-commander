#ifndef VISUALIZERWIDGET_H
#define VISUALIZERWIDGET_H

#include "visualizerprimitive.h"
#include "grblinstruction.h"
#include "grblstatus.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QMouseEvent>
#include <QVector3D>
#include <QVector>



class VisualizerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit VisualizerWidget(QWidget *parent = nullptr);
    ~VisualizerWidget();

signals:

public slots:

    void cleanModel(void);
    void appendPrimitive(int line, QVector<QVector3D> path, bool isWork = true);
    void onInstructionSent(GrblInstruction instruction);
    void onInstructionError(GrblInstruction instruction);
    void onInstructionOk(GrblInstruction instruction);
    void rewindModel(void);

    void onGrblStatusUpdated(GrblStatus* const status);

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;

    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

    void initShaders();

    void resetView();
    void setPathSegmentStatus(int line, VisualizerPrimitive::Status status);

private:
    QOpenGLShaderProgram* m_program;

    QVector2D m_mousePrevPosition;

    QVector3D m_translation;
    QQuaternion m_rotation;

    QMatrix4x4 m_projection;

    QVector<VisualizerPrimitive*> m_pathSegmentsVector;

    VisualizerPrimitive* m_drillPrimitive;
    VisualizerPrimitive* m_axisPrimitives[3];

    QVector3D m_drillPosition;
};

#endif // VISUALIZERWIDGET_H
