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
    configuration.cpp \
    privatemessagewindow.cpp \
    chatboxwidget.cpp \
    registrationdialog.cpp \
    partchanneldialog.cpp \
    linktextedit.cpp \
    filewidget.cpp

HEADERS  += mainwindow.hpp \
    logindialog.hpp \
    configuration.hpp \
    privatemessagewindow.hpp \
    chatboxwidget.hpp \
    registrationdialog.hpp \
    partchanneldialog.hpp \
    linktextedit.hpp \
    filewidget.hpp

FORMS    += mainwindow.ui \
    logindialog.ui \
    privatemessagewindow.ui \
    chatboxwidget.ui \
    registrationdialog.ui \
    partchanneldialog.ui \
    filewidget.ui
TRANSLATIONS += CTP_en.ts \
    CTP_bg.ts \
    CTP_ru.ts \
    CTP_de.ts
