
#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <iostream>

MoneyAvailable deposit;
QTimer main_timer, trend_adapt_timer;
unsigned int initial_money;

unsigned int main_timer_interval;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    initial_money = default_initial_money;

    seed(); // For the ticker symbols!

    ui->setupUi(this);
    ui->lcdMoney->hide();
    ui->initialMoney->setValue(default_initial_money);

    deposit.changeMoney(default_initial_money);

    QObject::connect(&deposit,SIGNAL( moneyChanged(int) ),ui->lcdMoney,SLOT(display(int)));
    QObject::connect(ui->startButton,SIGNAL( clicked() ),this,SLOT( startGame() ));
    QObject::connect(&reseed_timer,SIGNAL( timeout() ),this,SLOT( seed() ));
    QObject::connect(ui->speedBox,SIGNAL( valueChanged(int) ),this,SLOT( changeInterval(int)) );

    // Initialize reseed timer
    reseed_timer.setSingleShot(false);
    reseed_timer.setInterval(30e3);
    reseed_timer.start();

    // Initialize market change timer (trend_adapt_timer)
    trend_adapt_timer.setSingleShot(false);
    trend_adapt_timer.setInterval(100);
    trend_adapt_timer.start();

    // Set up main timer (controlling the update frequency of the prices)
    main_timer.setSingleShot(false);
    // Set timer interval to 400 / 8 = 50 ms
    ui->speedBox->setValue(8);
}

void MainWindow::seed(void)
{
    qsrand(QDateTime::currentMSecsSinceEpoch());

    //std::cout << "Reseed: " << QDateTime::currentMSecsSinceEpoch() << "\n";
    return;
}

void MainWindow::startGame(void)
{
    deposit.changeMoney(initial_money = ui->initialMoney->value());
    ui->initialMoney->hide();
    ui->lcdMoney->show();

    ui->startButton->setText("Pause");
    QObject::disconnect(ui->startButton,SIGNAL( clicked() ),this,SLOT( startGame() ));
    QObject::connect(ui->startButton,SIGNAL( clicked() ),this,SLOT( pauseGame() ));

    main_timer.start();

    return;
}

void MainWindow::pauseGame(void)
{
    main_timer.stop();

    ui->startButton->setText("Continue");

    QObject::disconnect(ui->startButton,SIGNAL( clicked() ),this,SLOT( pauseGame() ));
    QObject::connect(ui->startButton,SIGNAL( clicked() ),this,SLOT( continueGame() ));

}


void MainWindow::continueGame(void)
{
    main_timer.start();
    ui->startButton->setText("Pause");
    QObject::disconnect(ui->startButton,SIGNAL( clicked() ),this,SLOT( continueGame() ));
    QObject::connect(ui->startButton,SIGNAL( clicked() ),this,SLOT( pauseGame() ));
}

void MainWindow::changeInterval(int interval_divisor)
{
    main_timer_interval = max_interval / interval_divisor;

    main_timer.setInterval(main_timer_interval);
}

MainWindow::~MainWindow()
{
    afterGameFinished();

    delete ui;
}

void MainWindow::afterGameFinished(void)
{
    if ( deposit.getMoney() - initial_money > 0 )
        std::cout << "Congratulations! You earned " << deposit.getMoney() - initial_money << " units of liquid capital!\n";
    else if ( deposit.getMoney() - initial_money == 0)
    {
    }
    else
        std::cout << "Congratulations! You lost " << ((-1) * (deposit.getMoney() - initial_money)) << " units of liquid capital!\n";

    return;
}