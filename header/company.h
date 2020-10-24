#ifndef COMPANY_H
#define COMPANY_H

#include <QObject>
#include <localpricegen.h>

/*
 * Just again a quite misleading name: This class represents the price
 * of a company along with its representation in the user's depot.
 */
class Company
{
public:
    explicit Company(void);

    void initCompany(double ymax = 100);

    double getPrice(void);
    void recalcAvg(void);
    void buy(int);
    void sell(int);

    double updatePrice(void);

private:
    void split(void);

    LocalPriceGen price_generator;

    double current_price;
    int shares_in_depot;
    double total_value;
    double avg_depot_price;
    int ymax;
    bool is_bankrupt, splitted;

    friend class StockPriceHistoryPlot;
    friend class SingleStock;
};

#endif // COMPANY_H
