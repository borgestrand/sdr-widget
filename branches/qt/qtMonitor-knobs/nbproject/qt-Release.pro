# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Release/GNU-Linux-x86
TARGET = qtMonitor-knobs
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += release 
QT = core gui
SOURCES += audio.cpp main.cpp waterfall.cpp spectrum.cpp qtMonitor.cpp
HEADERS += waterfall.h qtMonitor.h spectrum.h audio.h
FORMS += qtMonitor.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Release/GNU-Linux-x86
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = gcc
QMAKE_CXX = g++
DEFINES += 
INCLUDEPATH += 
LIBS += 
