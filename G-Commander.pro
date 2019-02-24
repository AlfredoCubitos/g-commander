#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T23:16:28
#
#-------------------------------------------------

QT       += core gui serialport

CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = G-Commander
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    grblboard.cpp \
    grblstatus.cpp \
    grblinstruction.cpp \
    gcodestreamer.cpp \
    historymodel.cpp \
    historyitem.cpp \
    widgets/movementswidget.cpp \
    widgets/monitorwidget.cpp \
    widgets/controlwidget.cpp \
    widgets/hardwarewidget.cpp \
    widgets/gcodefilewidget.cpp \
    widgets/coordinatedisplay.cpp \
    widgets/visualizerwidget.cpp \
    widgets/visualizerprimitive.cpp \
    gcodeparser.cpp \
    grblerrorrecorder.cpp \
    grblconfigurationdialog.cpp \
    grblconfiguration.cpp

HEADERS  += mainwindow.h \
    grblboard.h \
    grblstatus.h \
    grbldefinitions.h \
    grblinstruction.h \
    gcodestreamer.h \
    historymodel.h \
    historyitem.h \
    widgets/movementswidget.h \
    widgets/monitorwidget.h \
    widgets/controlwidget.h \
    widgets/hardwarewidget.h \
    widgets/gcodefilewidget.h \
    widgets/coordinatedisplay.h \
    widgets/visualizerwidget.h \
    widgets/visualizerprimitive.h \
    gcodeparser.h \
    grblerrorrecorder.h \
    grblconfigurationdialog.h \
    grblconfiguration.h

FORMS    += \
    widgets/movementswidget.ui \
    widgets/monitorwidget.ui \
    widgets/controlwidget.ui \
    widgets/hardwarewidget.ui \
    widgets/gcodefilewidget.ui \
    widgets/coordinatedisplay.ui \
    grblconfigurationdialog.ui

RESOURCES += \
    icons.qrc \
    shaders.qrc
