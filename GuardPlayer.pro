#-------------------------------------------------
#
# Project created by QtCreator 2018-09-21T11:03:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GuardPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += "../../bin/ffmpeg-4.1.3/include"

SOURCES += \
        main.cpp \
        guardplayer.cpp \
        videowidget.cpp \
        videoframethread.cpp

HEADERS += \
        guardplayer.h \
        videowidget.h \
        videoframethread.h

LIBS += -L../../bin/ffmpeg-4.1.3/lib -lavformat -lavdevice -lavfilter -lavcodec -lswscale -lavutil -lswresample
LIBS += -lz
LIBS += -lm


