
#ifndef MONEYAVAILABLE_H
#define MONEYAVAILABLE_H

#include <QObject>

class MoneyAvailable : public QObject
{
    Q_OBJECT
public:
    explicit MoneyAvailable(QObject *parent = 0);
    double getMoney(void);

signals:
    void moneyChanged(int);

public slots:
    void changeMoney(double);

private:
    double money;

};

extern MoneyAvailable deposit;

#endif // MONEYAVAILABLE_H