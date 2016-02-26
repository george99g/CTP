#-------------------------------------------------
#
# Project created by QtCreator 2016-01-20T15:19:52
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CTP-client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    logindialog.cpp \
    configuration.cpp

HEADERS  += mainwindow.hpp \
    logindialog.hpp \
    configuration.hpp

FORMS    += mainwindow.ui \
    logindialog.ui
