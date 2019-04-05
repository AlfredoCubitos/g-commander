#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QSettings>
#include <QMenuBar>
#include <QScreen>

#include "grblboard.h"
#include "gcodestreamer.h"
#include "gcodeparser.h"
#include "grblerrorrecorder.h"

#include "widgets/controlwidget.h"
#include "widgets/coordinatedisplay.h"
#include "widgets/gcodefilewidget.h"
#include "widgets/hardwarewidget.h"
#include "widgets/monitorwidget.h"
#include "widgets/movementswidget.h"
#include "widgets/visualizerwidget.h"



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void showGrblSettingsDialog();


private slots:
    void onGrblError(GrblInstruction instruction, QString errorString);
    void onStreamerCompleted(void);
    void onGrblStatusUpdated(GrblStatus* const status);
    void onStreamerParsingCompleted();





protected:
    void showEvent(QShowEvent* e);
    void closeEvent(QCloseEvent *e);

private:
    void connectEverything();
    void setupDefaultDockLayout();
    void restoreSettings();
    void saveSettings();

    void addWidgetAndDockToUi(QDockWidget* dock,QWidget* widget);
    void createWidgets();


    GrblBoard *grbl;
    GCodeStreamer* streamer;
    GCodeParser* parser;

    QDockWidget* hardwareDock;
    HardwareWidget* hardwareWidget;

    QDockWidget* controlDock;
    ControlWidget* controlWidget;

    QDockWidget* gcodeFileDock;
    GCodeFileWidget* gcodeFileWidget;

    QDockWidget* movementsDock;
    MovementsWidget* movementsWidget;

    QDockWidget* monitorDock;
    MonitorWidget* monitorWidget;

    QDockWidget* positionDock;
    CoordinateDisplay* positionWidget;

    QDockWidget* visualizerDock;
    VisualizerWidget* visualizerWidget;

    //set Actions
    QAction *boardMenu;
    QAction *projectMenu;
    QAction *movementMenu;
    QAction *positionMenu;
    QAction *monitorMenu;
    QAction *visualizerMenu;

    QSettings *settings;

    QString createErrorSummary(GrblInstruction instruction, QString errorString);
    bool isGrblInCheckMode;
    QStringList m_errorSummaryList;



};

#endif // MAINWINDOW_H
