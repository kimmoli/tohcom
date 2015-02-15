#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "interrupt.h"

#define GPIO_INT "67"
#define GPIO_INT_EDGE "falling"

Interrupt::Interrupt(QObject *parent) :
    QObject(parent)
{
    m_abort = false;
    m_polling = false;
}

Interrupt::~Interrupt()
{
    abort();
}

void Interrupt::requestPolling()
{
    mutex.lock();

    m_polling = true;
    m_abort = false;
    m_gpio_fd = requestGpioInterrupt("67", "falling");

    mutex.unlock();

    printf("interrupt polling requested\n");

    emit pollingRequested();
}

void Interrupt::abort()
{
    mutex.lock();
    if (m_polling)
    {
        m_abort = true;
    }
    mutex.unlock();
}

void Interrupt::doPoll()
{
    struct pollfd fdset[1];
    int nfds = 1;

    int timeout;
    int dummy = 0;
    char *buf[20];

    timeout = 1000;

    for (;;)
    {

        // Checks if the process should be aborted
        mutex.lock();
        bool abort = m_abort;
        mutex.unlock();

        if (abort)
        {
            break;
        }

        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd = m_gpio_fd;
        fdset[0].events = POLLPRI;

        poll(fdset, nfds, timeout);

        if (fdset[0].revents & POLLPRI)
        {
            dummy += read(fdset[0].fd, buf, 20);
            emit interruptCaptured();
        }
    }

    mutex.lock();
    m_polling = false;
    releaseGpioInterrupt(m_gpio_fd, "67");
    mutex.unlock();

    emit finished();
}

int Interrupt::requestGpioInterrupt(const char *gpio, const char *edge)
{
    int fd;
    int retval = 0;

    fd = open("/sys/class/gpio/export", O_WRONLY);

    if (!(fd < 0))
    {
        retval += write (fd, gpio, strlen(gpio));
        close(fd);
    }

    fd = open(QString("/sys/class/gpio/gpio%1/edge").arg(gpio).toLocal8Bit().data(), O_WRONLY);

    if (!(fd < 0))
    {
        retval += write (fd, edge, strlen(edge));
        close(fd);
    }
    else
        return -1; /* error */

    fd = open(QString("/sys/class/gpio/gpio%1/edge").arg(gpio).toLocal8Bit().data(), O_RDONLY | O_NONBLOCK);

    return fd;
}

void Interrupt::releaseGpioInterrupt(int fd, const char* gpio)
{
    int retval = 0;

    close(fd);

    fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (!(fd < 0))
    {
        retval += write (fd, gpio, strlen(gpio));
        close(fd);
    }
}

