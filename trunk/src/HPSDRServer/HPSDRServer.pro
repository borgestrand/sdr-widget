#-------------------------------------------------
#
# Project created by QtCreator 2011-02-11T10:01:22
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = HPSDRServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

unix {
  LIBS += -lpcap
}

SOURCES += main.cpp \
    webserver.cpp \
    server.cpp \
    interfaces.cpp \
    webclient.cpp \
    discovery.cpp \
    metis.cpp \
    receiver.cpp \
    clientserver.cpp \
    client.cpp \
    audio.cpp \
    error.cpp

HEADERS += \
    webserver.h \
    server.h \
    interfaces.h \
    webclient.h \
    discovery.h \
    metis.h \
    receiver.h \
    clientserver.h \
    client.h \
    audio.h \
    error.h
