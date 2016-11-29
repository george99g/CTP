#-------------------------------------------------
#
# Project created by QtCreator 2016-01-20T15:19:52
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
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
    filewidget.cpp \
    usermodedialog.cpp \
    channelmodedialog.cpp \
    changepassworddialog.cpp

HEADERS  += mainwindow.hpp \
    logindialog.hpp \
    configuration.hpp \
    privatemessagewindow.hpp \
    chatboxwidget.hpp \
    registrationdialog.hpp \
    partchanneldialog.hpp \
    linktextedit.hpp \
    filewidget.hpp \
    usermodedialog.hpp \
    channelmodedialog.hpp \
    changepassworddialog.hpp

FORMS    += mainwindow.ui \
    logindialog.ui \
    privatemessagewindow.ui \
    chatboxwidget.ui \
    registrationdialog.ui \
    partchanneldialog.ui \
    filewidget.ui \
    usermodedialog.ui \
    channelmodedialog.ui \
    changepassworddialog.ui
TRANSLATIONS_SUBDIR = langs

TRANSLATIONS += $${TRANSLATIONS_SUBDIR}/CTP_en.ts \
    $${TRANSLATIONS_SUBDIR}/CTP_bg.ts

RESOURCES += \
    rsc.qrc
