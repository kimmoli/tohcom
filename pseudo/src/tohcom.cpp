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
#include "consolereader.h"

void signalHandler(int sig);

QCoreApplication* app;

int main(int argc, char *argv[])
{

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
        return EXIT_FAILURE;
    }

    umask(0);

    signal(SIGCHLD, signalHandler);
    signal(SIGTSTP, signalHandler);
    signal(SIGTTOU, signalHandler);
    signal(SIGTTIN, signalHandler);
    signal(SIGHUP,  signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);

    app = new QCoreApplication(argc, argv);

    setlinebuf(stdout);
    setlinebuf(stderr);

    if (argc < 2)
    {
        printf("tohcom version " APPVERSION " (C) kimmoli 2015\n\n");
        printf("Usage:\n");
        printf("tohcom {options,...}\n\n");
        printf(" -m              create /dev/pts\n");
        printf(" -t              do not init uart on start\n");
        printf(" -v              be verbose\n");
        printf(" -x              force stop of existing tohcom instance\n");

        return EXIT_FAILURE;
    }

    bool debugPrints = false;
    bool doComs = false;
    bool gotArgs = false;
    bool testMode = false;
    bool forceStop = false;

    for (int i=1; i<argc; i++)
    {
        if (QString(argv[i]).left(2) == "-v")
        {
            debugPrints = true;
        }
        else if (QString(argv[i]).left(2) == "-t")
        {
            testMode = true;
        }
        else if (QString(argv[i]).left(2) == "-x")
        {
            forceStop = true;
        }
        else if (QString(argv[i]).left(2) == "-m")
        {
            gotArgs = true;
            doComs = true;
        }
    }

    if (!gotArgs)
    {
        printf("Insuffucient arguments\n");
        return EXIT_FAILURE;
    }

    qint64 myPid = app->applicationPid();

    QFile* pidfile = new QFile("/tmp/tohcom.lock");

    if (pidfile->open(QIODevice::ReadOnly))
    {
        int pid = pidfile->readAll().toInt();
        pidfile->close();

        printf("tohcom is already running with pid %d\n", pid);

        if (!forceStop)
        {
            printf("use -x to force stopping of existing\n");
            return EXIT_FAILURE;
        }

        if (pid > 0 && kill(pid, 0) == 0)
        {
            printf("Sending SIGTERM to pid %d\n", pid);
            kill(pid, SIGTERM);
            QThread::msleep(500);
        }
        else
        {
            printf("Removing lockfile\n");
            pidfile->remove();
        }
    }

    pidfile->open(QIODevice::WriteOnly);
    pidfile->write(QByteArray(QString("%1").arg(myPid).toLatin1()));
    pidfile->close();

    pseudoport* port = new pseudoport();
    QThread* t_port = new QThread();
    port->moveToThread(t_port);

    i2ccoms* coms = new i2ccoms();
    QThread* t_coms = new QThread();
    coms->moveToThread(t_coms);

    ConsoleReader* console = new ConsoleReader();

    if (doComs)
    {
        app->connect(console, SIGNAL(uartDebugCommand(QString)), coms, SLOT(debugCommand(QString)), Qt::DirectConnection);
        app->connect(console, SIGNAL(wantsToQuit()), app, SLOT(quit()), Qt::DirectConnection);
        app->connect(coms, SIGNAL(debugCommandFinished(bool)), console, SLOT(prompt(bool)));

        port->debugPrints = debugPrints;
        coms->testMode = testMode;
        coms->debugPrints = debugPrints;

        /* Connect pseudoport Rx to I2C Tx and I2C Rx to pseudoport Tx */
        app->connect(port, SIGNAL(receive(QByteArray)), coms, SLOT(transmit(QByteArray)));
        app->connect(coms, SIGNAL(receive(QByteArray)), port, SLOT(transmit(QByteArray)));

        app->connect(port, SIGNAL(changeBaudrate(ulong)), coms, SLOT(changeBaudrate(ulong)));

        app->connect(t_coms, SIGNAL(started()), coms, SLOT(initComs()));
        app->connect(t_port, SIGNAL(started()), port, SLOT(create()));

        /* In case of coms error, just quit threads */
        app->connect(coms, SIGNAL(commsErrorFatal()), coms, SLOT(deleteLater()), Qt::DirectConnection);
        app->connect(coms, SIGNAL(destroyed()), port, SLOT(deleteLater()), Qt::DirectConnection);
        app->connect(coms, SIGNAL(destroyed()), t_coms, SLOT(quit()), Qt::DirectConnection);
        app->connect(port, SIGNAL(destroyed()), t_port, SLOT(quit()), Qt::DirectConnection);

        /* Quit application when pseudoport gets destroyed */
        app->connect(port, SIGNAL(destroyed()), app, SLOT(quit()), Qt::DirectConnection);

        t_coms->start();
        t_port->start();
    }
    
    int i = app->exec();

    pidfile->remove();

    printf("*stopping*\n");

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
