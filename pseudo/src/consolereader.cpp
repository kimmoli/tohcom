/*
 * thanks to http://stackoverflow.com/questions/6878507/using-qsocketnotifier-to-select-on-a-char-device/7389622#7389622
 */

#include "consolereader.h"
#include <QTextStream>

#include <unistd.h>

ConsoleReader::ConsoleReader(QObject *parent) :
    QObject(parent),
    notifier(STDIN_FILENO, QSocketNotifier::Read)
{
    connect(&notifier, SIGNAL(activated(int)), this, SLOT(text()));
}

void ConsoleReader::text()
{
    QTextStream qin(stdin);
    processCommandLine(qin.readLine());
}

void ConsoleReader::processCommandLine(QString line)
{
    if (line.isEmpty())
    {
        prompt(false);
        return;
    }

    if (line.startsWith("help", Qt::CaseInsensitive))
    {
        printf("help        show this help\n");
        printf("quit        quit tohcom\n");
    }
    else if (line.startsWith("quit", Qt::CaseInsensitive))
    {
        emit wantsToQuit();
        return;
    }

    emit uartDebugCommand(line);
}

void ConsoleReader::prompt(bool unknown)
{
    if (unknown)
        printf("Unknown command\n");
    printf("> ");
    fflush(stdout);
}
