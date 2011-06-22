# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Debug/GNU-Linux-x86
TARGET = qtMonitor
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += debug 
QT = core gui network
SOURCES += audio.cpp main.cpp waterfall.cpp spectrum.cpp qtMonitor.cpp
HEADERS += waterfall.h qtMonitor.h spectrum.h audio.h
FORMS += qtMonitor.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Debug/GNU-Linux-x86
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = gcc
QMAKE_CXX = g++
DEFINES += 
INCLUDEPATH += 
LIBS += -lQtMultimedia  
