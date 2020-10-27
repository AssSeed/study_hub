#ifndef GENERICPRICEGENERATOR_H
#define GENERICPRICEGENERATOR_H

#include <QObject>

class GenericPriceGenerator : public QObject
{
    Q_OBJECT
public:
    explicit GenericPriceGenerator(QObject *parent = 0);

    virtual void setRange(int ymax = 100) = 0;
    virtual int getRange(void) = 0;
    virtual double getPrice(void) = 0;

signals:

public slots:
    virtual void newTrendCoeff(void) = 0; // defined as {} for remote price generators.

};

#endif // GENERICPRICEGENERATOR_H
