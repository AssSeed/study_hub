#ifndef GENERICPRICEGENERATOR_H
#define GENERICPRICEGENERATOR_H

#include <QObject>

class GenericPriceGenerator : public QObject
{
    Q_OBJECT
public:
    explicit GenericPriceGenerator(QObject *parent = 0);

    virtual void setRange(int ymax = 100) = 0;
    virtual int getRange(void