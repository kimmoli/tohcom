#ifndef SC16IS850L_H
#define SC16IS850L_H

#include <QThread>

#include "driverBase.h"
#include "interrupt.h"

class SC16IS850L : public DriverBase
{
    Q_OBJECT
public:
    explicit SC16IS850L(unsigned char address);
    ~SC16IS850L();

    bool init();
    bool initOk;

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
