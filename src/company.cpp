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

    if (current_price <= 0.02 * yma