#include <moneyavailable.h>

MoneyAvailable::MoneyAvailable(QObject *parent) :
    QObject(parent),
    money(0)
{
}

double MoneyAvailable::getMoney(void)
{
    return money;
}

void MoneyAvailable::changeMoney(double m)
{
    money = m;
    emit moneyChanged((int)money);
  