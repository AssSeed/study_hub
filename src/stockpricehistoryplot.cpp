
#include <stockpricehistoryplot.h>
#include <company.h>
#include <iostream>

StockPriceHistoryPlot::StockPriceHistoryPlot(QWidget *parent) :
    QCustomPlot(parent)
{
    company.initCompany();
}

void StockPriceHistoryPlot::initCompanyPlot(int mx, double my)
{
    i = 0;
    xmax = mx;
    ymax = my;

    company.initCompany(ymax);

    y.fill(0,xmax+1);
    x.resize(xmax+1);
    avg.fill(0,2);
    avgx.fill(0,2);
    update_limit.fill(0,2);
    update_limitx.fill(0,2);

    update_limit[0] = 0;
    update_limit[0] = ymax;
    avgx[0] = 0;
    avgx[1] = xmax;

    for (int i = 0; i < xmax; i++)
    {
        x[i] = i;
    }

    initPlot();

    return;
}

void StockPriceHistoryPlot::initPlot(void)
{
    this->hide();

    this->removeGraph(0);
    this->removeGraph(1);
    this->removeGraph(2);
    this->addGraph();
    this->addGraph();
    this->addGraph();

    this->xAxis->setRange(0,xmax);
    this->xAxis->setTickLabels(false);
    this->yAxis->setRange(0,ymax);
    //this->yAxis->setLabel("Stock Price");

    this->graph(0)->setPen(QPen(Qt::red));
    this->graph(0)->setBrush(QBrush(QColor(255,0,0,30)));
    this->graph(1)->setPen(QPen(Qt::green));
    this->graph(2)->setPen(QPen(Qt::blue));

    this->show();

    return;
}

void StockPriceHistoryPlot::setData(void)
{
    if (i > xmax)
        i = 0;

    update_limitx[0] = update_limitx[1] = (i+1)%1000;
    double current_price = 0;

    // Random number calculation

    y[i] = current_price = company.updatePrice();

    i++;

    avg.fill(company.avg_depot_price,2); // Set (0,avg_price) and (xmax,avg_price) for the green line.

    this->graph(0)->setData(x,y);
    this->graph(1)->setData(avgx,avg);
    this->graph(2)->setData(update_limitx,update_limit);

    //std::cout << "Average price in dep: " << company.avg_depot_price << "\n";

    this->replot();

    if ( current_price == 0 )
        emit bankrupt();
    if ( company.splitted )
    {
        company.splitted = false;
        emit splitted();
    }
    emit priceChanged(current_price);

    return;
}