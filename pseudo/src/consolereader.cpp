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
    QString line = qin.readLine();

    if (line.isEmpty())
    {
        prompt();
        return;
    }

    if (line.startsWith("help", Qt::CaseInsensitive))
    {
        printf("help        show this help\n");
        printf("quit        quit tohcom\n");
        emit uartDebugCommand("help");
    }
    else if (line.startsWith("quit", Qt::CaseInsensitive))
    {
        emit wantsToQuit();
    }
    else if (line.startsWith("u", Qt::CaseInsensitive))
    {
        emit uartDebugCommand(line);
    }
    else
    {
        printf("Unknwon command: %s\n", qPrintable(line));
        prompt();
    }
}

void ConsoleReader::prompt()
{
    printf("> ");
    fflush(stdout);
}
