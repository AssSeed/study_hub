#include <company.h>
#include <localpricegen.h>
#include <mainwindow.h>


Company::Company(void) :
    current_price(0), shares_in_depot(0), total_value(0),
    ymax(0), is_bankrupt(false), splitted(false)
{
    QObject::connect(&trend_adapt_timer,SIGNAL( timeout() ),&price_generator,SLOT( newTrendCoeff() ));
}

void Company::initCompany(double my)
{
    is_bankrupt = false;
    price_generator.setRange(my);
    ymax = price_generator.getRange();

    return;
}

double Company::updatePrice(void)
{
    current_price = price_generator.getPrice();

    if (current_price <= 0.02 * ymax)
    {
        is_bankrupt = true;
        current_price = shares_in_depot = total_value = 0;
    }
    else if (current_price >= 0.97 * ymax)
    {
        split();
    }

    recalcAvg();

    return current_price;
}

double Company::getPrice(void)
{
    return current_price;
}

void Company::split(void)
{
    current_price /= 2;
    shares_in_depot *= 2;

    avg_depot_price = total_value / shares_in_depot;

    splitted = true;

    price_generator.setPrice(current_price);

    return;
}

void Company::buy(int n)
{
    shares_in_depot += n;
    total_value += n * current_price;

    recalcAvg();
    return;
}

void Company::sell(int n)
{
    total_value -= n * (total_value / shares_in_depot);
    shares_in_depot -= n;

    recalcAvg();
    return;
}

void Company::recalcAvg(void)
{
    if ( shares_in_depot > 0)
        avg_depot_price = total_value / shares_in_depot;
    else avg_depot_price = 0;

    return;
}
