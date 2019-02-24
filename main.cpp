#include "mainwindow.h"
#include <QApplication>

#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
   // QSurfaceFormat format;
  //  format.setDepthBufferSize(24);
  //  format.setSamples(16);
  //  QSurfaceFormat::setDefaultFormat(format);

    //Required so that context des not change chen docking / undocking visualizer
  //  QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    return a.exec();
}
