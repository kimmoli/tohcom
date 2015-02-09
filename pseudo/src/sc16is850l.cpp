#include "sc16is850l.h"
#include "sc16is850l_registers.h"
#include "driverBase.h"
#include <math.h>

SC16IS850L::SC16IS850L(unsigned char address)
{
    printf("start SC16IS850L, address 0x%02x\n", address);
    m_address = address;
    initOk = false;
}

SC16IS850L::~SC16IS850L()
{
    controlVdd(false);
    printf("close SC16IS850L\n");
}

bool SC16IS850L::init()
{
    /* Toggle power to make power-on reset for sure */

    controlVdd(false);
    QThread::msleep(50);

    controlVdd(true);
    QThread::msleep(350); /* RT9818C has max 297ms reset timeout */

    /* Check that the uart is found */

    if (readBytes(m_address, 1) == QByteArray())
    {
        /* Read result failed */
        printf("Failed to detect uart device\n");
        controlVdd(false);
        initOk = false;

        return false;
    }

    /* Arm interrupt */

    interrupt = new Interrupt();
    t_interrupt = new QThread(this);

    interrupt->moveToThread(t_interrupt);
    QObject::connect(interrupt, SIGNAL(pollingRequested()), t_interrupt, SLOT(start()));
    QObject::connect(t_interrupt, SIGNAL(started()), interrupt, SLOT(doPoll()));
    QObject::connect(interrupt, SIGNAL(finished()), t_interrupt, SLOT(quit()), Qt::DirectConnection);
    QObject::connect(interrupt, SIGNAL(interruptCaptured()), this, SLOT(processInterrupt()));

    interrupt->requestPolling();

    initOk = true;

    return true;
}


void SC16IS850L::processInterrupt()
{
    printf("got interrupt\n");
}

/* Calculate and set uart baudrate */
void SC16IS850L::setBaudrate(unsigned long bps, unsigned long xtal)
{
    double divisor = (double)xtal/(double)(bps*16.0);
    int N = (int)floor(divisor);
    double tmp;
    int M = (int)round( modf(divisor, &tmp) *16);
    double actual = (double)xtal/( 16.0*((double)N+(double)M/16.0) );

    printf("bps %lu xtal %lu\n", bps, xtal);
    printf("divisor %f N %d M %d\n", divisor, N, M);
    printf("actual %f\n", actual);
}


/* Control TOH VDD */
void SC16IS850L::controlVdd(bool state)
{
    int fd;
    int retval = 0;

    fd = open("/sys/devices/platform/reg-userspace-consumer.0/state", O_WRONLY);

    if (!(fd < 0))
    {
        retval += write (fd, state ? "1" : "0", 1);
        close(fd);
    }
}
