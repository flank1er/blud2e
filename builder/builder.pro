#-------------------------------------------------
#
# Project created by QtCreator 2015-03-10T11:37:21
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = builder
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    aboutdlg.cpp \
    ../blud2e/io.cpp \
    ../blud2e/conver.cpp \
    ../blud2e/obj.cpp

LIBS += -L/usr/lib64 -lGLU -lglut -lz

CONFIG += debug

QMAKE_CXXFLAGS += -std=c++11
#-DGL_GLEXT_PROTOTYPES

HEADERS  += mainwindow.h \
    glwidget.h \
    aboutdlg.h \
    ../blud2e/blud2e.h

FORMS    += mainwindow.ui \
    aboutdlg.ui

OTHER_FILES += \
    shaders/light.frag.glsl \
    shaders/light.vert.glsl

RESOURCES += \
    res/resourses.qrc
