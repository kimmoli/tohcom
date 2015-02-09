#include <QStringList>

#include <stdio.h>

#include "i2ccoms.h"

i2ccoms::i2ccoms(QObject *parent) :
    QObject(parent), uart(NULL)
{
    testMode = false;
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
    /* This should transmit 'data' over i2c to uart */
    printf("%s", data.data());
    fflush(stdout);

    /* just echo back */
    emit receive(data);
}

/* Debug interface */
void i2ccoms::debugCommand(QString cmd)
{
    if (cmd.startsWith("help", Qt::CaseInsensitive))
    {
        printf("udump       dump uart registers\n");
        printf("ureset      soft-reset uart\n");
        printf("uinit       initialize uart\n");
        printf("ukill       shutdown uart\n");
        printf("ubaud       set baudrate {bps(115200)} {xtal(25000000)}\n");
    }
    else if (cmd.startsWith("udump", Qt::CaseInsensitive))
    {
        printf("dump uart registers\n");
    }
    else if (cmd.startsWith("ureset", Qt::CaseInsensitive))
    {
        printf("soft reset uart\n");
    }
    else if (cmd.startsWith("ukill", Qt::CaseInsensitive))
    {
        delete(uart);
        uart = NULL;
    }
    else if (cmd.startsWith("uinit", Qt::CaseInsensitive))
    {
        delete(uart);
        uart = new SC16IS850L(0x4A);

        if (uart->init())
            printf("init success\n");
        else
            printf("init failed\n");
    }
    else if (cmd.startsWith("ubaud", Qt::CaseInsensitive))
    {
        QStringList p = cmd.split(" ");

        if (uart)
        {
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
    else
    {
        printf("Unknwon command: %s\n", qPrintable(cmd));
    }

    emit debugCommandFinished();
}

