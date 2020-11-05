
#ifndef STOCKPRICEHISTORYPLOT_H
#define STOCKPRICEHISTORYPLOT_H

#include <QVector>
#include <qcustomplot.h>
#include <company.h>

/*
 * This class is the price diagram in a SingleStock widget.
 * It also owns the company depot.

   The current price is fetched from a price generator, e.g. LocalPriceGen
   but maybe also some multi-player network price generator.
*/
class StockPriceHistoryPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit StockPriceHistoryPlot(QWidget *parent = 0);