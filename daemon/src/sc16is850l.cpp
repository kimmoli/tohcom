#include "sc16is850l.h"
#include "driverBase.h"
#include <math.h>

SC16IS850L::SC16IS850L(unsigned char address) : m_address(address)
{
    printf("start SC16IS850L, address 0x%02x\n", address);

    baud = 115200;
    parity = PARITY_NONE;
    stop = STOP_1;
    wordlen = WORDLEN_8;
    dtr = true;

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

    setBaudrate();
    setLineparams();

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
    QByteArray isr;

    do
    {
        isr = writeThenRead(m_address, GR_ISR, 1);

        if (isr.isEmpty())
        {
            printf("error reading ISR\n");
            return;
        }

        printf("got interrupt: ");
        switch (isr.at(0) & 0x3f)
        {
            case 0x06:
                printf("LSR\n");
                break;
            case 0x04:
                printf("RXRDY ready\n");
                break;
            case 0x0c:
                printf("RXRDY timeout\n");
                break;
            case 0x02:
                printf("TXRDY\n");
                break;
            case 0x00:
                printf("MSR\n");
                break;
            case 0x10:
                printf("RXRDY Xoff/special\n");
                break;
            case 0x20:
                printf("CTS/RTS change\n");
                break;
            default:
                if ((isr.at(0) & 0x01) == 0x00)
                    printf("unknown %02x\n", isr.at(0));
                break;
        }
    } while ((isr.at(0) & 0x01) == 0x00);
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

    QByteArray lcr = writeThenRead(m_address, GR_LCR, 1);
    if (lcr.isEmpty())
        return;

    printf("lcr = %02x\n", lcr.at(0));

    /* Enable Special register set */
    writeBytes(m_address, QByteArray().append(GR_LCR).append(lcr.at(0) | 0x80));
    /* Write DLL and DLM */
    writeBytes(m_address, QByteArray().append(SR_DLL).append((char)(N & 0xff)));
    writeBytes(m_address, QByteArray().append(SR_DLM).append((char)((N >> 8) & 0xff)));
    /* Restore General register set */
    writeBytes(m_address, QByteArray().append(GR_LCR).append(lcr.at(0) & (~0x80)));

    /* Enable Second extra feature register set */
    writeBytes(m_address, QByteArray().append(GR_EFCR).append(EFCR_SEFR));
    /* Write fractional divider to CLKPRES */
    writeBytes(m_address, QByteArray().append(SEFR_CLKPRES).append(M & 0x0f));
    /* Restore General register set */
    writeBytes(m_address, QByteArray().append(GR_EFCR).append(EFCR_GR));
}

/* Set line parameters, parity, stop bits and word length */
void SC16IS850L::setLineparams(int parity, int stop, int wordlen)
{
    writeBytes(m_address, QByteArray().append(GR_LCR).append(parity | stop | wordlen));

    this->stop = stop;
    this->parity = parity;
    this->wordlen = wordlen;
}

void SC16IS850L::setDtr(bool state)
{
    dtr = state;

    QByteArray mcrRead = writeThenRead(m_address, GR_MCR, 1);
    if (mcrRead.isEmpty())
        return;

    char mcr = mcrRead.at(0);

    if (dtr)
        mcr &= ~MCR_DTRL;
    else
        mcr |= MCR_DTRL;

    writeBytes(m_address, QByteArray().append(GR_MCR).append(mcr));
}

/* Transmit bytes */
void SC16IS850L::transmit(QByteArray data)
{
    QByteArray lsr;
    for (int i=0 ; i<data.count() ; i++)
    {
        do
        {
            lsr = writeThenRead(m_address, GR_LSR, 1);
            if (lsr.isEmpty())
                return;
        }
        while ((lsr.at(0) & LSR_THREMPTY) != LSR_THREMPTY);

        if (!writeBytes(m_address, QByteArray().append(GR_THR).append(data.at(i))))
            return;
    }
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
