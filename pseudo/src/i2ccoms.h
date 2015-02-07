#ifndef I2CCOMS_H
#define I2CCOMS_H

#include <QObject>

class i2ccoms : public QObject
{
    Q_OBJECT
public:
    explicit i2ccoms(QObject *parent = 0);

signals:

public slots:
    void received(char c);

};

#endif // I2CCOMS_H
