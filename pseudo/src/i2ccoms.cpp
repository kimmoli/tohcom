#include <stdio.h>

#include "i2ccoms.h"

i2ccoms::i2ccoms(QObject *parent) :
    QObject(parent)
{
}

i2ccoms::~i2ccoms()
{
}

void i2ccoms::transmit(QByteArray data)
{
    /* This should transmit 'data' over i2c to uart */
    printf("%s", data.data());
    fflush(stdout);

    /* just echo back */
    emit receive(data);
}
