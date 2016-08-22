#-------------------------------------------------
#
# Project created by QtCreator 2016-08-22T22:27:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = openwith-qt
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    ../../source/basedir.cpp \
    ../../source/desktopfile.cpp \
    ../../source/inilike.cpp \
    ../../source/path.cpp \
    ../../source/system.cpp

HEADERS  += widget.h \
    ../../source/basedir.h \
    ../../source/desktopfile.h \
    ../../source/inilike.h \
    ../../source/mimeapps.h \
    ../../source/path.h \
    ../../source/splitter.h \
    ../../source/system.h
