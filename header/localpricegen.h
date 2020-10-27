
#ifndef LOCALPRICEGEN_H
#define LOCALPRICEGEN_H

#include <QObject>
#include <genericpricegenerator.h>

class LocalPriceGen : public GenericPriceGenerator
{
    Q_OBJECT
public:
    explicit LocalPriceGen(QObject *parent = 0);

    void setRange(int ymax = 100);
    int getRange(void);
    double getPrice(void);
    void setPrice(double);

public slots:
    void newTrendCoeff(void);

private:
    int ymax;
    double current_price;
    double trend_coeff;

};

#endif // LOCALPRICEGEN_H