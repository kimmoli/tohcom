#
# Project TOHCOM
#

TARGET = picocom

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT -= gui
QT += dbus

DEFINES += VERSION_STR=\\\"1.7.2\\\"
DEFINES += HIGH_BAUD

target.path = /usr/bin/

desktop.path = /usr/share/applications
desktop.files = config/tohcom.desktop

icon.path = /usr/share/icons/hicolor/86x86/apps/
icon.files = config/tohcom.png

INSTALLS += target desktop icon

SOURCES += \
    src/term.c \
    src/picocom.cpp
	
HEADERS += src/term.h \
    src/picocom.h

OTHER_FILES += \
    config/tohcom.desktop \
    config/tohcom.png \
