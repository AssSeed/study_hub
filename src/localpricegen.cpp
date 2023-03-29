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
        v