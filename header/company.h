#ifndef COMPANY_H
#define COMPANY_H

#include <QObject>
#include <localpricegen.h>

/*
 * Just again a quite misleading name: This class represents the price
 * of a company along with its representation in the user's depot.
 */
class Company
{
public:
    explicit Company(void);

    void initCompany(double ymax = 100);

    double getPric