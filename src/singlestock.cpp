#include <singlestock.h>
#include <ui_singlestock.h>
#include <mainwindow.h>
#include <QTimer>
#include <company.h>

int xmax = 600;

SingleStock::SingleStock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleStock),
    buy_step(1)
{
    ui->setupUi(this);

    ui->plot->initCompanyPlot(xmax,100);

    QObject::connect(ui->plot,SIGNAL( priceChanged(int) ),ui->lcdPrice,SLOT( display(int) ));
    QObject::connect(&main_timer,SIGNAL( timeout() ),ui->plot,S