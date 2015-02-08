#ifndef CONSOLEREADER_H
#define CONSOLEREADER_H

#include <QObject>
#include <QSocketNotifier>

class ConsoleReader : public QObject
{
    Q_OBJECT
public:
    explicit ConsoleReader(QObject *parent = 0);

signals:
    void uartDebugCommand(QString cmd);
    void wantsToQuit();

public slots:
    void text();
    void prompt();

private:
    QSocketNotifier notifier;
};

#endif // CONSOLEREADER_H
