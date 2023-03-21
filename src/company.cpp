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
    is_bankrupt = fa