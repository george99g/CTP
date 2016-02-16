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
    irc/ircconnections.cpp \
    irc/ircmanager.cpp \
    irc/ircchannel.cpp \
    irc/ircchannels.cpp \
    irc/ircmode.cpp \
    irc/ircclient.cpp \
    irc/ircclients.cpp

HEADERS  += dialog.hpp \
    tcp/tcpconnection.hpp \
    tcp/tcpconnections.hpp \
    tcp/tcpserver.hpp \
    irc/ircserver.hpp \
    irc/ircconnections.hpp \
    irc/ircmanager.hpp \
    irc/ircchannel.hpp \
    irc/ircchannels.hpp \
    irc/ircmode.hpp \
    irc/ircclient.hpp \
    irc/ircclients.hpp \
    config.hpp

FORMS    += dialog.ui
