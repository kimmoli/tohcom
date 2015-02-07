#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QProcessEnvironment>
#include <QThread>

#include "pseudoport.h"
#include "i2ccoms.h"

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

    QCoreApplication app(argc, argv);

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

            QObject::connect(port, SIGNAL(received(char)), coms, SLOT(received(char)));
            printf("connected some signals to slots\n");
        }
    }
    
    return app.exec();
}

