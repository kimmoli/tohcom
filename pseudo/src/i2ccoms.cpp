#include <QStringList>

#include <stdio.h>

#include "i2ccoms.h"

i2ccoms::i2ccoms(QObject *parent) :
    QObject(parent), uart(NULL)
{
    testMode = false;
    debugPrints = false;
}

i2ccoms::~i2ccoms()
{
    delete(uart);
    printf("close i2ccoms\n");
}

void i2ccoms::initComs()
{
    printf("start i2ccoms\n");

    if (!testMode)
    {
        uart = new SC16IS850L(0x4A);

        if (!uart->init())
        {
            printf("uart init failed\n");
            emit commsErrorFatal();
        }
    }
    else
    {
        printf("testmode, not initialising uart\n");
    }
}

void i2ccoms::transmit(QByteArray data)
{
    /* Transmit 'data' over i2c to uart */

    if (debugPrints)
    {
        printf("%s", data.data());
        fflush(stdout);
    }

    if (uart)
        uart->transmit(data);

    /* just echo back */
    emit receive(data);
}

void i2ccoms::changeBaudrate(unsigned long bps)
{
    if (uart)
        uart->setBaudrate(bps);
}

/* Debug interface */
void i2ccoms::debugCommand(QString cmd)
{
    bool unknown = false;

    if (cmd.startsWith("help", Qt::CaseInsensitive))
    {
        printf("dump        dump uart registers\n");
        printf("reset       soft-reset uart\n");
        printf("init        initialize uart\n");
        printf("kill        shutdown uart\n");
        printf("baud        set baudrate {bps(115200)} {xtal(25000000)}\n");
        printf("send        send string over uart {string ...}\n");
    }
    else if (cmd.startsWith("dump", Qt::CaseInsensitive))
    {
        printf("dump uart registers\n");
    }
    else if (cmd.startsWith("reset", Qt::CaseInsensitive))
    {
        printf("soft reset uart\n");
    }
    else if (cmd.startsWith("kill", Qt::CaseInsensitive))
    {
        delete(uart);
        uart = NULL;
    }
    else if (cmd.startsWith("init", Qt::CaseInsensitive))
    {
        delete(uart);
        uart = new SC16IS850L(0x4A);

        if (uart->init())
            printf("init success\n");
        else
            printf("init failed\n");
    }
    else if (cmd.startsWith("baud", Qt::CaseInsensitive))
    {
        if (uart)
        {
            QStringList p = cmd.split(" ");

            if (p.count() == 1)
                uart->setBaudrate();
            else if (p.count() == 2)
                uart->setBaudrate(p.at(1).toLong());
            else if (p.count() > 2)
                uart->setBaudrate(p.at(1).toLong(), p.at(2).toLong());
        }
        else
            printf("uart not initialized\n");
    }
    else if (cmd.startsWith("send", Qt::CaseInsensitive))
    {
        if (uart)
        {
            QStringList p = cmd.split(" ");
            p.removeFirst();
            if (!p.empty())
            {
                uart->transmit(p.join(" ").toLocal8Bit());
            }
        }
        else
            printf("uart not initialized\n");
    }
    else
    {
        unknown = true;
    }

    emit debugCommandFinished(unknown);
}
