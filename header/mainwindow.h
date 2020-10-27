#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QTimer>

#include "stockpricehistoryplot.h"
#include "moneyavailable.h"

const int default_initial_money = 10000;
const int max_interval = 400;
extern unsigned i