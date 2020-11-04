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
 * The company behind the graph is place