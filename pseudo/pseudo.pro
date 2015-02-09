#
# Project TOHCOM
#

TARGET = tohcom

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT -= gui

target.path = /usr/bin/

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

message($${DEFINES})

INSTALLS += target

SOURCES += src/tohcom.cpp \
    src/driverBase.cpp \
    src/pseudoport.cpp \
    src/i2ccoms.cpp \
    src/sc16is850l.cpp \
    src/interrupt.cpp \
    src/consolereader.cpp
	
HEADERS += src/driverBase.h \
    src/pseudoport.h \
    src/i2ccoms.h \
    src/sc16is850l.h \
    src/interrupt.h \
    src/consolereader.h \
    src/sc16is850l_registers.h
