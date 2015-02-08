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
        return;

    if (line.startsWith("quit", Qt::CaseInsensitive))
    {
        emit wantsToQuit();
    }
    else if (line.startsWith("help", Qt::CaseInsensitive))
    {
        printf("yeah sure there is help\n");
    }
    else if (line.startsWith("u", Qt::CaseInsensitive))
    {
        emit uartDebugCommand(line);
    }
    else
    {
        printf("Unknwon command: %s\n", qPrintable(line));
    }
}
