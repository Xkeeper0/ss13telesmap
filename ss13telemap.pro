#-------------------------------------------------
#
# Project created by QtCreator 2013-03-26T05:43:30
#
#-------------------------------------------------

QT       += core gui widgets

QMAKE_CXXFLAGS += -std=c++11
CONFIG += c++11

TARGET = ss13telemap
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    calibrationdialog.cpp \
    mapview.cpp \
    usagedialog.cpp

HEADERS  += mainwindow.h \
    calibrationdialog.h \
    mapview.h \
    usagedialog.h

FORMS    += mainwindow.ui \
    calibrationdialog.ui \
    usagedialog.ui
