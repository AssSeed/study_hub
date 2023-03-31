#include <localpricegen.h>
#include <QDateTime>

LocalPriceGen::LocalPriceGen(QObject *parent) :
    GenericPriceGenerator(parent),
    ymax(100),
    current_price(ymax/2),
    trend_coeff(4.5)
{
}

void LocalPriceGen::setRange(int y)
{
    ymax = y;
    current_price = ymax/2;

    return;
}

int LocalPriceGen::getRange(void)
{
    return ymax;
}

double getRandomDivisor(void)
{
    return 1 + (qrand() % 5);
}

// This is the core algorithm to produce good-looking stock price diagrams!
double LocalPriceGen::getPrice(void)
{
    int threshold = 0.05 * ymax;
    double value = current_price;

    if ( value < threshold )
        value += ((qrand() % 10) - 3)/getRandomDivisor();
    else if ( value > (ymax - threshold) )
        value += ((qrand() % 10) - 7)/getRandomDivisor();
    else value += ((qrand() % 10) - trend_coeff)/getRandomDivisor();

    current_price = value;

    return value;
}

void LocalPriceGen::setPrice(double p)
{
    current_price = p;

    return;
}

void LocalPriceGen::newTrendCoeff(void)
{
    trend_coeff = 4.1 + 0.2 * (qrand()%5);

    return;
}
