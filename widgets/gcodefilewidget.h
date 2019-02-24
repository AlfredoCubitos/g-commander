#ifndef GCODEFILEWIDGET_H
#define GCODEFILEWIDGET_H

#include <QTime>
#include <QWidget>


#include "gcodestreamer.h"

namespace Ui {
class GCodeFileWidget;
}

class GCodeFileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GCodeFileWidget(QWidget *parent = 0);
    ~GCodeFileWidget();

signals:
    void openFile(QString filePath);
    void go();
    void stop();
    void rewind();
    void step();
    void goToLine(int line);

public slots:
    void onFileLoaded(QString filename);
    void onStreamerStateChanged(GCodeStreamer::states state);
    void onStreamerLineCountChanged(int line);
    void onStreamerLineParsedChanged(int line);
    void onEstimatedDurationUpdated(uint32_t duration);

private slots:
    void onOpenButtonClicked();
    void onCurrentLineSpinBoxEditingFinished();


private:
    Ui::GCodeFileWidget *ui;
};

#endif // GCODEFILEWIDGET_H
