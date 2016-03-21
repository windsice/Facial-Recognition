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
    detector.cpp \
    stillobject.cpp \
    XML_creator.cpp

unix {
    INCLUDEPATH += /usr/local/include/opencv
    LIBS += `pkg-config opencv --libs`
}

win32 {
    INCLUDEPATH += C://software//opencv//MyBuild//install//include
    LIBS += -LC://software//opencv//MyBuild//bin \
        libopencv_calib3d249d \
        libopencv_contrib249d \
        libopencv_core249d \
        libopencv_features2d249d \
        libopencv_flann249d \
        libopencv_gpu249d \
        libopencv_highgui249d \
        libopencv_imgproc249d \
        libopencv_legacy249d \
        libopencv_ml249d \
        libopencv_nonfree249d \
        libopencv_objdetect249d \
        libopencv_ocl249d \
        libopencv_photo249d \
        libopencv_stitching249d \
        libopencv_superres249d \
        libopencv_video249d \
        libopencv_videostab249d \
}


HEADERS  += recognizer.h \
    detector.h \
    stilldetection.h \
    stillobject.h \
    XML_creator.h

FORMS    += recognizer.ui \
    detector.ui \
    stillobject.ui \
    XML_creator.ui

RESOURCES += \
    Icon.qrc
