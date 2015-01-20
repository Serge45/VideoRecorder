#-------------------------------------------------
#
# Project created by QtCreator 2014-12-22T12:02:48
#
#-------------------------------------------------

QT       += core gui concurrent network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
TARGET = VideoRecorder
TEMPLATE = app

OBJECTS_DIR = $${PWD}/obj
UI_DIR = $${PWD}/generatedFiles/uic

debug: {
    MOC_DIR = $${PWD}/generatedFiles/moc/debug
}

release: {
    MOC_DIR = $${PWD}/generatedFiles/moc/release
}

INCLUDEPATH += $${PWD}/include

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L$${PWD}/lib/debug \
                -lopencv_core249d \
                -lopencv_highgui249d \
                -lopencv_imgproc249d \
    }

    CONFIG(release, debug|release){
        LIBS += -L$${PWD}/release \
                -lopencv_core249 \
                -lopencv_highgui249 \
                -lopencv_imgproc249 \
    }
}

unix {
    LIBS += -L/usr/local/lib \
            -lopencv_core \
            -lopencv_highgui \
            -lopencv_imgproc \
}

SOURCES += main.cpp\
        mainwindow.cpp \
    cameradevice.cpp \
    controlpanel.cpp \
    cvmatandqimage.cpp \
    highprecisiontimer.cpp \
    hovermenubutton.cpp \
    imageview.cpp \
    videorecorder.cpp \
    VideoRecorderIPCCommand.cpp \
    geometryengine.cpp \
    glimageview.cpp

HEADERS  += mainwindow.h \
    cameradevice.h \
    controlpanel.h \
    cvmatandqimage.h \
    highprecisiontimer.h \
    hovermenubutton.h \
    imageview.h \
    videorecorder.h \
    VideoRecorderGlobalDef.h \
    VideoRecorderIPCCommand.h \
    geometryengine.h \
    glimageview.h

FORMS    += mainwindow.ui \
    controlpanel.ui \
    imageview.ui \
    videorecorder.ui

RESOURCES += \
    videorecorder.qrc \
    shaders.qrc
