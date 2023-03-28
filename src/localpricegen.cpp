#include <localpricegen.h>
#include <QDateTime>

LocalPriceGen::LocalPriceGen(QObject *parent) :
    GenericPriceGenerator(parent),
    ymax(10