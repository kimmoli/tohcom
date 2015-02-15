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

DEFINES += "APPVERSION=\\\"$${SPECVERSION}\\\""

target.path = /usr/bin/

dbusconf.path = /etc/dbus-1/system.d/
dbusconf.files = config/tohcom.conf

dbusservice.path = /usr/share/dbus-1/system-services/
dbusservice.files = config/com.kimmoli.tohcom.service

INSTALLS += target dbusconf dbusservice

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
    config/com.kimmoli.tohcom.xml \
    config/com.kimmoli.tohcom.service \
    config/tohcom.conf
