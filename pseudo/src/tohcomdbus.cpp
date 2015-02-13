#include "tohcomdbus.h"

#include <stdio.h>

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";

TohcomDBus::TohcomDBus(QObject *parent) :
    QObject(parent)
{
    m_dbusRegistered = false;
}

TohcomDBus::~TohcomDBus()
{
    if (m_dbusRegistered)
    {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterObject(PATH);
        connection.unregisterService(SERVICE);

        printf("TohcomDBus: unregistered from dbus sessionBus\n");
    }
}

void TohcomDBus::registerDBus()
{
    if (!m_dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::sessionBus();
        if (!connection.registerService(SERVICE))
        {
            QCoreApplication::quit();
            return;
        }

        if (!connection.registerObject(PATH, this))
        {
            QCoreApplication::quit();
            return;
        }
        m_dbusRegistered = true;

        printf("TohcomDBus: succesfully registered to dbus sessionBus \"%s\"\n", SERVICE);
    }
}

void TohcomDBus::command(const QString &line)
{
    if (!line.isEmpty())
        emit commandFromDbus(line);
}

QString TohcomDBus::getVersion()
{
    return QString(APPVERSION);
}
