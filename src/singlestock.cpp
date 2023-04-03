#include <singlestock.h>
#include <ui_singlestock.h>
#include <mainwindow.h>
#include <QTimer>
#include <company.h>

int xmax = 600;

SingleStock::SingleStock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleStock),
    