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
        uart = new SC16IS850L(0x4D);

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
        printf("show        show current line settings\n");
        printf("baud        set baudrate bps {xtal(18432000)}\n");
        printf("send        send string over uart {string ...}\n");
        printf("parity      set parity: none, even, odd\n");
        printf("bits        set word length: 5, 6, 7 ,8\n");
        printf("stop        set stop bits: 1, 2\n");
        printf("flow        set flow-control: none, RTS/CTS, xon/xoff\n");
        printf("dtr         control dtr pin: pulse, up, down\n");
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
        uart = new SC16IS850L(0x4D);

        if (uart->init())
            printf("init success\n");
        else
            printf("init failed\n");
    }
    else if (cmd.startsWith("show", Qt::CaseInsensitive))
    {
        if (uart)
            printf("%d,%s,%d,%d %s\n", uart->baud,
                   (uart->parity == PARITY_NONE) ? "n" : ((uart->parity == PARITY_EVEN) ? "e" : "o"),
                   uart->wordlen + 5,
                   uart->stop == STOP_1 ? 1 : 2,
                   uart->initOk ? "" : "(not initialized properly)");
        else
            printf("uart not initialized\n");

    }
    else if (cmd.startsWith("baud", Qt::CaseInsensitive))
    {
        if (uart)
        {
            QStringList p = cmd.split(" ");

            if (p.count() == 2)
                uart->setBaudrate(p.at(1).toLong());
            else if (p.count() == 3)
                uart->setBaudrate(p.at(1).toLong(), p.at(2).toLong());
            else
                printf("Baudrate is %d\n", uart->baud);
        }
        else
            printf("uart not initialized\n");
    }
    else if (cmd.startsWith("parity", Qt::CaseInsensitive))
    {
        if (uart)
        {
            QStringList p = cmd.split(" ");

            if (p.count() == 2)
            {
                if (p.at(1).startsWith("n"))
                    uart->setLineparams(PARITY_NONE, uart->stop, uart->wordlen);
                else if (p.at(1).startsWith("e"))
                    uart->setLineparams(PARITY_EVEN, uart->stop, uart->wordlen);
                else if (p.at(1).startsWith("o"))
                    uart->setLineparams(PARITY_ODD, uart->stop, uart->wordlen);
                else
                    printf("illegal value\n");
            }
            else
                printf("Parity is %s\n", (uart->parity == PARITY_NONE) ? "none"
                                        : ((uart->parity == PARITY_EVEN) ? "even" : "odd"));
        }
        else
            printf("uart not initialized\n");
    }
    else if (cmd.startsWith("bits", Qt::CaseInsensitive))
    {
        if (uart)
        {
            QStringList p = cmd.split(" ");

            if (p.count() == 2)
            {
                if (p.at(1).toInt() == 5)
                    uart->setLineparams(uart->parity, uart->stop, WORDLEN_5);
                else if (p.at(1).toInt() == 6)
                    uart->setLineparams(uart->parity, uart->stop, WORDLEN_6);
                else if (p.at(1).toInt() == 7)
                    uart->setLineparams(uart->parity, uart->stop, WORDLEN_7);
                else if (p.at(1).toInt() == 8)
                    uart->setLineparams(uart->parity, uart->stop, WORDLEN_8);
                else
                    printf("illegal value\n");
            }
            else
                printf("Wordlength is %d\n", uart->wordlen + 5);
        }
        else
            printf("uart not initialized\n");
    }
    else if (cmd.startsWith("stop", Qt::CaseInsensitive))
    {
        if (uart)
        {
            QStringList p = cmd.split(" ");

            if (p.count() == 2)
            {
                if (p.at(1).toInt() == 1)
                    uart->setLineparams(uart->parity, STOP_1, uart->wordlen);
                else if (p.at(1).toInt() == 2)
                    uart->setLineparams(uart->parity, STOP_2, uart->wordlen);
                else
                    printf("illegal value\n");
            }
            else
                printf("Stop bits %d\n", uart->stop == STOP_1 ? 1 : 2);
        }
        else
            printf("uart not initialized\n");
    }
    else if (cmd.startsWith("flow", Qt::CaseInsensitive))
    {
        printf("flow-control not implemented\n");
    }
    else if (cmd.startsWith("dtr", Qt::CaseInsensitive))
    {
        if (uart)
        {
            QStringList p = cmd.split(" ");

            if (p.count() == 2)
            {
                if (p.at(1).startsWith("p"))
                {
                    uart->setDtr(!uart->dtr);
                    QThread::msleep(1000);
                    uart->setDtr(!uart->dtr);
                }
                else if (p.at(1).startsWith("u"))
                    uart->setDtr(true);
                else if (p.at(1).startsWith("d"))
                    uart->setDtr(false);
                else
                    printf("illegal value\n");
            }
            else
                printf("dtr is %s\n", uart->dtr ? "up" : "down");
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
