#include "mainwindow.h"
#include "grbldefinitions.h"
#include "grblconfigurationdialog.h"

#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    isGrblInCheckMode = false;

    setWindowTitle("G-Commander");

    setCentralWidget(nullptr);

    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    grbl = new GrblBoard(this);

    streamer = new GCodeStreamer(this);

    parser = new GCodeParser(this);

   resize(QSize(640,480));
   createWidgets();


    settings = new QSettings("settings.ini",QSettings::IniFormat,this);

    connectEverything();

}


void MainWindow::setupDefaultDockLayout(){

    splitDockWidget(hardwareDock,gcodeFileDock,Qt::Vertical);
    splitDockWidget(gcodeFileDock,controlDock,Qt::Vertical);
    splitDockWidget(controlDock,monitorDock,Qt::Vertical);
    tabifyDockWidget(monitorDock,movementsDock);
    splitDockWidget(positionDock,visualizerDock,Qt::Vertical);

}

void MainWindow::connectEverything(){

    //GRBL - streamer related connections
    connect(grbl,&GrblBoard::ok,                streamer,&GCodeStreamer::onInstructionParsedByGrbl);
    connect(grbl,&GrblBoard::statusUpdated,     streamer,&GCodeStreamer::onGrblStatusUpdated);
    connect(grbl,&GrblBoard::instructionSent,   streamer,&GCodeStreamer::onInstructionSentToGrbl);
    connect(grbl,&GrblBoard::boardStartup,      streamer,&GCodeStreamer::stop);

    connect(streamer,&GCodeStreamer::instructionToSend,   grbl,&GrblBoard::sendInstruction);

    connect(grbl,&GrblBoard::error,         this,&MainWindow::onGrblError);
    connect(grbl,&GrblBoard::statusUpdated, this,&MainWindow::onGrblStatusUpdated);

    connect(streamer,&GCodeStreamer::workCompleted,this,&MainWindow::onStreamerCompleted);
    connect(streamer,&GCodeStreamer::lineCountUpdated,this,&MainWindow::onStreamerParsingCompleted);

    //Parser
    connect(streamer,&GCodeStreamer::instructionLoaded,parser,&GCodeParser::parseInstruction);
    connect(streamer,&GCodeStreamer::cleared,parser,&GCodeParser::reset);


}

void MainWindow::addWidgetAndDockToUi(QDockWidget *dock, QWidget *widget){
    //widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    dock->setWidget(widget);
    dock->setAllowedAreas(Qt::TopDockWidgetArea);
    this->addDockWidget(Qt::TopDockWidgetArea,dock);
}


void MainWindow::createWidgets(){

     QMenu *showMenu = menuBar()->addMenu(tr("&Show"));

    hardwareDock = new QDockWidget("Board", this);
    hardwareDock->setObjectName("DockWidget");
    hardwareWidget = new HardwareWidget(hardwareDock);
    addWidgetAndDockToUi(hardwareDock,hardwareWidget);
    showMenu->addAction(hardwareDock->toggleViewAction());

    connect(grbl,&GrblBoard::statusUpdated,hardwareWidget,&HardwareWidget::onGrblStatusUpdated);
    connect(hardwareWidget,&HardwareWidget::serialSettingsUpdated,grbl,&GrblBoard::setSerialSettings);
    connect(hardwareWidget,&HardwareWidget::toggleSerial,grbl,&GrblBoard::toggleSerial);
    connect(hardwareWidget,&HardwareWidget::grblSettingsButtonPressed,this,&MainWindow::showGrblSettingsDialog);

    hardwareWidget->onGrblStatusUpdated(grbl->getLastStatus());



    controlDock = new QDockWidget("Controls", this);
    controlDock->setObjectName("ControlDock");
    controlWidget = new ControlWidget(controlDock);
    addWidgetAndDockToUi(controlDock,controlWidget);
    showMenu->addAction(controlDock->toggleViewAction());

    connect(controlWidget,&ControlWidget::cmdReset,grbl,&GrblBoard::rtCmdSoftReset);
    connect(controlWidget,&ControlWidget::cmdPause,grbl,&GrblBoard::rtCmdPauseCycle);
    connect(controlWidget,&ControlWidget::cmdResume,grbl,&GrblBoard::rtCmdResumeCycle);
    connect(controlWidget,&ControlWidget::sendInstruction,grbl,&GrblBoard::sendInstruction);
    connect(grbl,&GrblBoard::statusUpdated,controlWidget,&ControlWidget::onGrblStatusUpdated);

    controlWidget->onGrblStatusUpdated(grbl->getLastStatus());


    gcodeFileDock = new QDockWidget("GCode Project", this);
    gcodeFileDock->setObjectName("GcodeFileDock");
    showMenu->addAction(gcodeFileDock->toggleViewAction());

    gcodeFileWidget = new GCodeFileWidget(gcodeFileDock);
    addWidgetAndDockToUi(gcodeFileDock,gcodeFileWidget);

    connect(gcodeFileWidget,&GCodeFileWidget::openFile, streamer,&GCodeStreamer::loadFile);
    connect(gcodeFileWidget,&GCodeFileWidget::go,       streamer,&GCodeStreamer::go);
    connect(gcodeFileWidget,&GCodeFileWidget::stop,     streamer,&GCodeStreamer::stop);
    connect(gcodeFileWidget,&GCodeFileWidget::rewind,   streamer,&GCodeStreamer::rewind);
    connect(gcodeFileWidget,&GCodeFileWidget::step,     streamer,&GCodeStreamer::step);
    connect(gcodeFileWidget,&GCodeFileWidget::goToLine, streamer,&GCodeStreamer::goToLine);
    connect(streamer,&GCodeStreamer::fileLoaded,                gcodeFileWidget,&GCodeFileWidget::onFileLoaded);
    connect(streamer,&GCodeStreamer::lineCountUpdated,          gcodeFileWidget,&GCodeFileWidget::onStreamerLineCountChanged);
    connect(streamer,&GCodeStreamer::currentLineUpdated, gcodeFileWidget,&GCodeFileWidget::onStreamerLineParsedChanged);
    connect(streamer,&GCodeStreamer::stateChanged,              gcodeFileWidget,&GCodeFileWidget::onStreamerStateChanged);

    gcodeFileWidget->onStreamerStateChanged(streamer->getState());

    movementsDock = new QDockWidget("Movements", this);
    movementsDock->setObjectName("MovementsDock");
    movementsWidget = new MovementsWidget(movementsDock);
    addWidgetAndDockToUi(movementsDock,movementsWidget);
    showMenu->addAction(movementsDock->toggleViewAction());

    connect(movementsWidget, &MovementsWidget::instructionToGrbl, grbl, &GrblBoard::sendInstruction);
    connect(grbl,&GrblBoard::statusUpdated,movementsWidget,&MovementsWidget::onGrblStatusUpdated);
    movementsWidget->onGrblStatusUpdated(grbl->getLastStatus());

    monitorDock = new QDockWidget("Monitor", this);
    monitorDock->setObjectName("MonitorDock");
    monitorWidget = new MonitorWidget(monitorDock);
    addWidgetAndDockToUi(monitorDock,monitorWidget);
    showMenu->addAction(monitorDock->toggleViewAction());
    connect(grbl,&GrblBoard::instructionSent,monitorWidget,&MonitorWidget::onInstructionSentToGrbl);
    connect(grbl,&GrblBoard::ok,monitorWidget,&MonitorWidget::onOkReceived);
    connect(grbl,&GrblBoard::error,monitorWidget,&MonitorWidget::onErrorReceived);
    connect(grbl,&GrblBoard::feedback,monitorWidget,&MonitorWidget::onFeedbackReceived);
    connect(grbl,&GrblBoard::text,monitorWidget,&MonitorWidget::onFeedbackReceived);
    connect(grbl,&GrblBoard::alarm,monitorWidget,&MonitorWidget::onAlarmReceived);
    connect(grbl,&GrblBoard::boardStartup,monitorWidget,&MonitorWidget::onBoardStartup);
    connect(grbl,&GrblBoard::statusUpdated,monitorWidget,&MonitorWidget::onGrblStatusUpdated);
    connect(monitorWidget,&MonitorWidget::sendInstruction,grbl,&GrblBoard::sendInstruction);
    monitorWidget->onGrblStatusUpdated(grbl->getLastStatus());

    positionDock = new QDockWidget("Position", this);
    positionDock->setObjectName("PositionDock");
    positionWidget = new CoordinateDisplay(positionDock);
    connect(grbl,&GrblBoard::statusUpdated,positionWidget,&CoordinateDisplay::onGrblStatusUpdated);
    connect(positionWidget,&CoordinateDisplay::instructionToGrbl,grbl,&GrblBoard::sendInstruction);
    addWidgetAndDockToUi(positionDock,positionWidget);
    showMenu->addAction(positionDock->toggleViewAction());

    visualizerDock = new QDockWidget("Visualizer", this);
    visualizerDock->setObjectName("VisualizerDock");
    visualizerWidget = new VisualizerWidget(visualizerDock);
    showMenu->addAction(visualizerDock->toggleViewAction());
    connect(streamer,&GCodeStreamer::cleared,visualizerWidget,&VisualizerWidget::cleanModel);
    connect(grbl,&GrblBoard::instructionSent,visualizerWidget,&VisualizerWidget::onInstructionSent);
    connect(grbl,&GrblBoard::ok,visualizerWidget,&VisualizerWidget::onInstructionOk);
    connect(grbl,&GrblBoard::error,visualizerWidget,&VisualizerWidget::onInstructionError);
    connect(gcodeFileWidget,&GCodeFileWidget::rewind,visualizerWidget,&VisualizerWidget::rewindModel);
    connect(grbl,&GrblBoard::statusUpdated,visualizerWidget,&VisualizerWidget::onGrblStatusUpdated);

    connect(parser,&GCodeParser::parsedPrimitive,visualizerWidget,&VisualizerWidget::appendPrimitive);

    addWidgetAndDockToUi(visualizerDock,visualizerWidget);

}


void MainWindow::showGrblSettingsDialog(){
    GrblConfigurationDialog dialog(this);

    connect(grbl,&GrblBoard::parametersMapUpdated,&dialog,&GrblConfigurationDialog::onParameterMapUpdated);
    connect(grbl,&GrblBoard::ok,&dialog,&GrblConfigurationDialog::onInstructionOk);
    connect(grbl,&GrblBoard::error,&dialog,&GrblConfigurationDialog::onInstructionError);
    connect(&dialog,&GrblConfigurationDialog::sendToGrbl,grbl,&GrblBoard::sendInstruction);

    dialog.exec();
}




void MainWindow::onStreamerCompleted(){
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Information");
    msgBox.setIcon(QMessageBox::Information);

    if(isGrblInCheckMode){
            switch(m_errorSummaryList.size()){
            case 0:
                msgBox.setText("Simulation completed without error");
                break;
            case 1:
                msgBox.setText("Simulation completed with 1 error");
                msgBox.setDetailedText(m_errorSummaryList.join('\n'));
                break;
            default:
                msgBox.setText(QString("Simulation completed with %1 errors").arg(m_errorSummaryList.size()));
                msgBox.setDetailedText(m_errorSummaryList.join('\n'));
            }
    }
    else{
        msgBox.setText("Work Completed !");
    }

    m_errorSummaryList.clear();

    msgBox.exec();
}

void MainWindow::onGrblStatusUpdated(GrblStatus* const status){
    GrblStatus::states currGrblState = status->getState();
    GrblStatus::states prevGrblState = status->getPreviousState();

    //Just entered check mode
    if(currGrblState == GrblStatus::state_check && prevGrblState != GrblStatus::state_check){
        m_errorSummaryList.clear();
        isGrblInCheckMode = true;
    }

    //Just left check mode
    else if(currGrblState != GrblStatus::state_check && prevGrblState == GrblStatus::state_check){
        m_errorSummaryList.clear();
        isGrblInCheckMode = false;
    }
}

void MainWindow::onStreamerParsingCompleted(){
    gcodeFileWidget->onEstimatedDurationUpdated(parser->getMachineTime());
}


void MainWindow::onGrblError(GrblInstruction instruction, QString errorString){
    QString errorSummary = createErrorSummary(instruction,errorString);

    if(isGrblInCheckMode){
        //Add error to list, so we can display error summary later
        m_errorSummaryList.append(errorSummary);
    }
    else if(streamer->getState() == GCodeStreamer::state_running){
        grbl->rtCmdPauseCycle();

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Error");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Grbl detected an error. \"Feed Hold\" send to avoid potential dangerous behavoir");
        msgBox.setDetailedText(errorSummary);
        msgBox.exec();
    }
}


QString MainWindow::createErrorSummary(GrblInstruction instruction, QString errorString){
    QString errorSummaryString("%1    %2"); //No need for a line return char, since instruction comes with a line return
    return errorSummaryString.arg(instruction.getStringWithLineNumber(),errorString);
}


void MainWindow::restoreSettings(){
    settings->beginGroup("MainWindow");

    move(           settings->value( "Pos",         pos()           ).toPoint());
    resize(         settings->value( "Size",        size()          ).toSize());
    restoreGeometry(settings->value( "Geometry",    saveGeometry()  ).toByteArray());
    restoreState(   settings->value( "State",       saveState()     ).toByteArray());


    if ( settings->value( "Maximized", isMaximized() ).toBool() ){
        showMaximized();
    }


    settings->endGroup();
}

void MainWindow::saveSettings(){
    settings->beginGroup("MainWindow");

    settings->setValue("Geometry",saveGeometry());
    settings->setValue("State",saveState());
    settings->setValue("Maximized", isMaximized() );
    if ( !isMaximized() ) {
        settings->setValue( "Pos", pos() );
        settings->setValue( "Size", size() );
    }

    settings->endGroup();
}

void MainWindow::showEvent(QShowEvent *e){
    setupDefaultDockLayout();
    restoreSettings();
    QMainWindow::showEvent(e);
}

void MainWindow::closeEvent(QCloseEvent *e){
    saveSettings();
    QMainWindow::closeEvent(e);
}

