#ifndef I2CCOMS_H
#define I2CCOMS_H

#include <QObject>

class i2ccoms : public QObject
{
    Q_OBJECT
public:
    explicit i2ccoms(QObject *parent = 0);
    ~i2ccoms();

signals:
    void receive(QByteArray data);

public slots:
    void transmit(QByteArray data);

};

#endif // I2CCOMS_H
