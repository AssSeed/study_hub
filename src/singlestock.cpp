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
    QObject::connect(&main_timer,SIGNAL( timeout() ),ui->plot,SLOT( setData()));
    QObject::connect(ui->buyButton,SIGNAL( clicked() ),this,SLOT( buyStock() ));
    QObject::connect(ui->sellButton,SIGNAL( clicked() ),this,SLOT( sellStock() ));
    QObject::connect(ui->orderStep,SIGNAL( valueChanged(int) ),this,SLOT( changeBuyStep(int) ));

    // Financial signals
    QObject::connect(ui->plot,SIGNAL( bankrupt(void) ),this,SLOT( bankrupt(void) ));
    QObject::connect(ui->plot,SIGNAL( splitted(void) ),this,SLOT( split(void) ));

    setCompanyName();
}

void SingleStock::changeBuyStep(int n)
{
    buy_step = n;

    return;
}

void SingleStock::buyStock(void)
{
    double current_price = ui->plot->company.getPrice();
    double order_volume = buy_step * current_price;

    if (ui->plot->company.is_bankrupt || ! main_timer.isActive() || deposit.getMoney() - order_volume < 0)
        return;

    deposit.changeMoney(deposit.getMoney()-order_volume);

    ui->plot->company.buy(buy_step);

    ui->lcdStocks->display(ui->plot->company.shares_in_depot);

    return;
}

void SingleStock::sellStock(void)
{

    if (ui->plot->company.is_bankrupt || ! main_timer.isActive() || ui->plot->company.shares_in_depot - buy_step < 0)
        return;

    double current_price = ui->plot->company.getPrice();
    double order_volume = buy_step * current_price;

    deposit.changeMoney(deposit.getMoney()+order_volume);

    ui->plot->company.sell(buy_step);

    ui->lcdStocks->display(ui->plot->company.shares_in_depot);

    return;
}

// Only updates the LCD displays
void SingleStock::split(void)
{
    QPalette Pal;
    Pal.setColor(QPalette::Background,Qt::green);
    ui->lcdPrice->setAutoFillBackground(true);
    ui->lcdPrice->setPalette(Pal);

    QTimer::singleShot(60*main_timer_interval,this,SLOT( clearPriceBG() ));

    ui->lcdStocks->display(ui->plot->company.shares_in_depot);

    return;
}

void SingleStock::bankrupt(void)
{
    ui->lcdStocks->display(0);
    ui->lcdPrice->display(0);
    ui->lcdPrice->setAutoFillBackground(true);

    QObject::disconnect(&main_timer,SIGNAL( timeout() ),ui->plot,SLOT( setData()));

    QPalette Pal;
    Pal.setColor(QPalette::Background,Qt::red);
    ui->lcdPrice->setAutoFillBackground(true);
    ui->lcdPrice->setPalette(Pal);

    // A QTimer object would be destructed before firing!
    // Time to wait before placing a new company on this stock position
    QTimer::singleShot(60*main_timer_interval,this,SLOT( reInit()));

    return;
}

void SingleStock::reInit(void)
{
    ui->plot->initCompanyPlot(xmax,100);
    setCompanyName();

    clearPriceBG();

    QObject::connect(&main_timer,SIGNAL( timeout() ),ui->plot,SLOT( setData()));

    return;
}

void SingleStock::clearPriceBG(void)
{
    ui->lcdPrice->setAutoFillBackground(false);

    return;
}

void SingleStock::setCompanyName(void)
{
    QString pool = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString name = "___";


    for ( int i = 0; i < 3; i++ )
    {
        name[i] = pool[qrand()%25];
    }

    