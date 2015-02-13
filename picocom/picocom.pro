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
# DEFINES += UUCP_LOCK_DIR=\\\"/var/lock\\\"
DEFINES += HIGH_BAUD
DEFINES += TOHCOM

message($${DEFINES})

target.path = /usr/bin/

INSTALLS += target

SOURCES += src/picocom.c \
    src/term.c
	
HEADERS += src/term.h

