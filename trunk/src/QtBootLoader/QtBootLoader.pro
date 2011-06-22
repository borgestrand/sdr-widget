#-------------------------------------------------
#
# Project created by QtCreator 2010-11-30T14:23:40
#
#-------------------------------------------------

QT       += core gui network

TARGET = QtBootLoader
TEMPLATE = app

win32 {
    LIBS += wpcap.lib
} else {
    LIBS += -lpcap
}

SOURCES += main.cpp\
        MainWindow.cpp \
    Interfaces.cpp \
    RawReceiveThread.cpp

HEADERS  += MainWindow.h \
    Interfaces.h \
    RawReceiveThread.h

FORMS    += MainWindow.ui
