#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QProcessEnvironment>
#include <QThread>
#include <QFile>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pseudoport.h"
#include "i2ccoms.h"

void signalHandler(int sig);

QCoreApplication* app;

int main(int argc, char *argv[])
{
#ifdef REQUIRE_ROOT
    bool rootUser = false;

    QStringList environment = QProcessEnvironment::systemEnvironment().toStringList();
    for (int n=0; n<environment.length(); n++)
        if (environment.at(n) == "USER=root")
        {
            rootUser = true;
            break;
        }

    if (!rootUser)
    {
        printf("Error: You need to be root to use this utility!\n");
        return 0;
    }
#endif

    umask(0);

    signal(SIGCHLD, signalHandler);
    signal(SIGTSTP, signalHandler);
    signal(SIGTTOU, signalHandler);
    signal(SIGTTIN, signalHandler);
    signal(SIGHUP,  signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);

    app = new QCoreApplication(argc, argv);

    qint64 myPid = app->applicationPid();

    QFile* pidfile = new QFile("/tmp/tohcom.lock");

    if (pidfile->open(QIODevice::ReadOnly))
    {
        printf("tohcom is already running with pid %s\n", pidfile->readAll().data());
        return 0;
    }
    else
    {
        pidfile->open(QIODevice::WriteOnly);
        pidfile->write(QByteArray(QString("%1").arg(myPid).toLatin1()));
        pidfile->close();
    }

    setlinebuf(stdout);
    setlinebuf(stderr);

    if (argc < 2)
    {
        printf("tohcom version " APPVERSION " (C) kimmoli 2015\n\n");
        printf("Usage:\n");
        printf("tohcom {options...}\n\n");
        printf(" -m              test /dev/pts\n");
        printf(" -x              do nothing\n");
        return 0;
    }

    pseudoport* port = new pseudoport();
    QThread* t_port = new QThread();
    port->moveToThread(t_port);

    i2ccoms* coms = new i2ccoms();
    QThread* t_coms = new QThread();
    coms->moveToThread(t_coms);

    for (int i=1; i<argc; i++)
    {
        if (QString(argv[i]).left(2) == "-x")
        {
            printf("Doing nothing.\n");
        }
        else if (QString(argv[i]).left(2) == "-m")
        {
            QObject::connect(t_port, SIGNAL(started()), port, SLOT(create()));

            t_coms->start();
            t_port->start();

            QThread::msleep(10);

            QObject::connect(port, SIGNAL(receive(QByteArray)), coms, SLOT(transmit(QByteArray)));
            QObject::connect(coms, SIGNAL(receive(QByteArray)), port, SLOT(transmit(QByteArray)));
            printf("ready.\n");
        }
    }
    
    int i = app->exec();

    pidfile->remove();

    printf("stopping.\n");

    return i;
}

void signalHandler(int sig)
{
    switch(sig)
    {
        case SIGHUP:
        case SIGTERM:
        case SIGINT:
            app->quit();
            break;
        default:
            printf("Received signal %d\n", sig);
            break;
    }
}
