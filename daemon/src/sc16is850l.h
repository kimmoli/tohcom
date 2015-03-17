#ifndef SC16IS850L_H
#define SC16IS850L_H

#include <QThread>

#include "driverBase.h"
#include "interrupt.h"
#include "sc16is850l_registers.h"

class SC16IS850L : public DriverBase
{
    Q_OBJECT
public:
    explicit SC16IS850L(unsigned char address);
    ~SC16IS850L();

    bool init();
    bool initOk;

    void setBaudrate(unsigned long bps = 115200, unsigned long xtal = 18432000);
    void setLineparams(int parity = PARITY_NONE, int stop = STOP_1, int wordlen = WORDLEN_8);
    void setDtr(bool state);
    void transmit(QByteArray data);

    int baud;
    int parity;
    int stop;
    int wordlen;
    bool dtr;

signals:

public slots:
    void processInterrupt();

private:
    void controlVdd(bool state);

    unsigned char m_address;

    QThread* t_interrupt;
    Interrupt* interrupt;
};

#endif // SC16IS850L_H
