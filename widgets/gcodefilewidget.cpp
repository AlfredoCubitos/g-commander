#include "gcodefilewidget.h"
#include "ui_gcodefilewidget.h"

#include <QFileDialog>

GCodeFileWidget::GCodeFileWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GCodeFileWidget)
{
    ui->setupUi(this);

    ui->currentLineSpinBox->setKeyboardTracking(false);

    connect(ui->openButton,&QPushButton::clicked,this,&GCodeFileWidget::onOpenButtonClicked);
    connect(ui->currentLineSpinBox,&QSpinBox::editingFinished,this,&GCodeFileWidget::onCurrentLineSpinBoxEditingFinished);

    connect(ui->goButton,&QPushButton::clicked,this,&GCodeFileWidget::go);
    connect(ui->stopButton,&QPushButton::clicked,this,&GCodeFileWidget::stop);
    connect(ui->rewindButton,&QPushButton::clicked,this,&GCodeFileWidget::rewind);
    connect(ui->stepButton,&QPushButton::clicked,this,&GCodeFileWidget::step);
}

void GCodeFileWidget::onOpenButtonClicked(){
    QString filter = QStringLiteral("G-Code files *.gc *.ngc");
    QString filepath = QFileDialog::getOpenFileName(this,tr("Select a file"),QString(),filter);
    if(!filepath.isEmpty()){
        emit openFile(filepath);
    }
}

void GCodeFileWidget::onCurrentLineSpinBoxEditingFinished(){
    if(ui->currentLineSpinBox->isEnabled()){
        emit goToLine(ui->currentLineSpinBox->value());
    }
}

void GCodeFileWidget::onFileLoaded(QString filename){
    ui->fileNameLabel->setText(filename);
}

void GCodeFileWidget::onStreamerStateChanged(GCodeStreamer::states state){
    switch(state){
    case GCodeStreamer::state_clear:
        ui->openButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        ui->goButton->setEnabled(false);
        ui->rewindButton->setEnabled(false);
        ui->stepButton->setEnabled(false);
        ui->currentLineSpinBox->setEnabled(false);
        break;
    case GCodeStreamer::state_ready:
        ui->openButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        ui->goButton->setEnabled(true);
        ui->rewindButton->setEnabled(true);
        ui->stepButton->setEnabled(true);
        ui->currentLineSpinBox->setEnabled(true);
        break;
    case GCodeStreamer::state_running:
        ui->openButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->goButton->setEnabled(false);
        ui->rewindButton->setEnabled(false);
        ui->stepButton->setEnabled(false);
        ui->currentLineSpinBox->setEnabled(false);
        break;
    }
}

void GCodeFileWidget::onStreamerLineParsedChanged(int line){
    //Do not change if has focus, so that the user can enter the line
    if(!ui->currentLineSpinBox->hasFocus()){
        ui->currentLineSpinBox->setValue(line);
    }

    ui->progressBar->setValue(line);
}

void GCodeFileWidget::onEstimatedDurationUpdated(uint32_t duration){
    QString durationString("%1h %2m %3s");
    duration /= (1000); //Convert ms to s
    ui->durationLabel->setText(durationString.arg(QString::number(duration/3600),
                                                  QString::number((duration/60)%60),
                                                  QString::number(duration%60)));
}

void GCodeFileWidget::onStreamerLineCountChanged(int line){
    ui->currentLineSpinBox->setMinimum((line>0) ? 1 : 0);
    ui->currentLineSpinBox->setMaximum(line);
    ui->progressBar->setMinimum((line>0) ? 1 : 0);
    ui->progressBar->setMaximum(line);
    ui->lineCountLabel->setText(QStringLiteral("%1 lines").arg(line));
}



GCodeFileWidget::~GCodeFileWidget()
{
    delete ui;
}


