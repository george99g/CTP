#-------------------------------------------------
#
# Project created by QtCreator 2016-01-09T20:11:24
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CTP-server
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    tcp/tcpconnection.cpp \
    tcp/tcpconnections.cpp \
    tcp/tcpserver.cpp \
    irc/ircserver.cpp \
    irc/ircconnection.cpp \
    irc/ircconnections.cpp

HEADERS  += dialog.hpp \
    tcp/tcpconnection.hpp \
    tcp/tcpconnections.hpp \
    tcp/tcpserver.hpp \
    irc/ircserver.hpp \
    irc/ircconnection.hpp \
    irc/ircconnections.hpp

FORMS    += dialog.ui
