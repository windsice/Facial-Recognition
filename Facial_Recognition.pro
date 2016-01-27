#-------------------------------------------------
#
# Project created by QtCreator 2014-11-12T21:31:25
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Facial_Recognition
TEMPLATE = app

CONFIG += mobility
MOBILITY = multimedia
SOURCES += main.cpp\
        recognizer.cpp \
    detector.cpp

INCLUDEPATH += /usr/local/include/opencv
LIBS += `pkg-config opencv --libs`



HEADERS  += recognizer.h \
    detector.h \
    stilldetection.h

FORMS    += recognizer.ui \
    detector.ui

RESOURCES += \
    Icon.qrc
