#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QTimer>

#include "stockpricehistoryplot.h"
#include "moneyavailable.h"

const int default_initial_money = 10000;
const int max_interval = 400;
extern unsigned int main_timer_interval;

extern QTimer main_timer, trend_adapt_timer;

namespace Ui {
class MainWindow;
}

/*
 * Not only the main window, but also responsible for the current
 * state of the game (running/paused)
 */

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void startGame(void);
    void pauseGame(void);
    void continueGame(void);
    void seed(void);
    void changeInterval