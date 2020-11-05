#ifndef SINGLESTOCK_H
#define SINGLESTOCK_H

#include <QWidget>
#include <QTimer>

#include <stockpricehistoryplot.h>

namespace Ui {
class SingleStock;
}

/*
 * This is the UI class consisting the Buy/Sell buttons, the price LCDs,
 * the price diagram and the buy step spin box.
 * The company behind the graph is placed in the plot widget (ui->plot)
 */
class SingleStock : public QWidget
{
    Q_OBJECT

public:
    explicit SingleStock(QWidget *parent = 0);
    ~SingleStock();

public slots:
    void buyStock(void);
    void sellStock(void);

private slots:
    void changeBuyStep(int)