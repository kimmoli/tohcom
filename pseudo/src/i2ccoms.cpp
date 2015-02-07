#include <stdio.h>

#include "i2ccoms.h"

i2ccoms::i2ccoms(QObject *parent) :
    QObject(parent)
{
    printf("i2ccoms started\n");
}

void i2ccoms::received(char c)
{
    printf("%c\n", c);
}
