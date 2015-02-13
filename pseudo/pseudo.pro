#
# Project TOHCOM
#

TARGET = tohcom

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT -= gui
QT += dbus

system(qdbusxml2cpp config/com.kimmoli.tohcom.xml -i src/tohcomdbus.h -a src/adaptor)

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
    src/consolereader.cpp \
    src/tohcomdbus.cpp \
    src/adaptor.cpp
	
HEADERS += src/driverBase.h \
    src/pseudoport.h \
    src/i2ccoms.h \
    src/sc16is850l.h \
    src/interrupt.h \
    src/consolereader.h \
    src/sc16is850l_registers.h \
    src/tohcomdbus.h \
    src/adaptor.h

OTHER_FILES += \
    config/com.kimmoli.tohcom.xml
