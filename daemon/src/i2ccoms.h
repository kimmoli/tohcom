#ifndef I2CCOMS_H
#define I2CCOMS_H

#include <QObject>
#include "sc16is850l.h"

class i2ccoms : public QObject
{
    Q_OBJECT
public:
    explicit i2ccoms(QObject *parent = 0);
    ~i2ccoms();
    bool testMode;
    bool debugPrints;

signals:
    void receive(QByteArray data);
    void commsErrorFatal();
    void debugCommandFinished(bool unknown);

public slots:
    void initComs(bool override = false);
    void transmit(QByteArray data);
    void debugCommand(QString cmd);
    void changeBaudrate(unsigned long bps);
    void uartReceive(QByteArray data);

private:
    SC16IS850L* uart;
};

#endif // I2CCOMS_H
