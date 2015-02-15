/*
 * thanks to http://stackoverflow.com/questions/6878507/using-qsocketnotifier-to-select-on-a-char-device/7389622#7389622
 */

#include "consolereader.h"
#include <QTextStream>

#include <unistd.h>

ConsoleReader::ConsoleReader(bool readConsole, QObject *parent) :
    m_readConsole(readConsole),
    QObject(parent),
    notifier(STDIN_FILENO, QSocketNotifier::Read)
{
    if (m_readConsole)
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
        if (m_readConsole)
            prompt(false);
        return;
    }

    if (line.startsWith("help", Qt::CaseInsensitive) && m_readConsole)
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
    if (m_readConsole)
    {
        if (unknown)
            printf("Unknown command\n");
        printf("> ");
        fflush(stdout);
    }
}
