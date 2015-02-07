#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QProcessEnvironment>
#include <QThread>


int main(int argc, char *argv[])
{
    bool fileNameGiven = false;
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

    if (argc < 2)
    {
        printf("tohcom version " APPVERSION " (C) kimmoli 2015\n\n");
        printf("Usage:\n");
        printf("tohcom {options...}\n\n");
        printf(" -x              do nothing\n");
        return 0;
    }

//    Stm32p* stm32 = new Stm32p();

    for (int i=1; i<argc; i++)
    {
        if (QString(argv[i]).left(2) == "-x")
        {
            printf("Doing nothing.\n");
        }
    }
    
    return 1;
}

