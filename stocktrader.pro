
#-------------------------------------------------
#
# Project created by QtCreator 2013-11-30T12:14:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = stocktrader
TEMPLATE = app

SOURCES += src/main.cpp\
    src/mainwindow.cpp \
    lib/qcustomplot.cpp \
    src/stockpricehistoryplot.cpp \
    src/singlestock.cpp \
    src/moneyavailable.cpp \
    src/localpricegen.cpp \
    src/genericpricegenerator.cpp \
    src/company.cpp

HEADERS  +=\
    header/mainwindow.h \
    lib/qcustomplot.h \
    header/stockpricehistoryplot.h \
    header/singlestock.h \
    header/moneyavailable.h \
    header/localpricegen.h \
    header/genericpricegenerator.h \
    header/company.h

FORMS    += mainwindow.ui \
    singlestock.ui

OTHER_FILES += \
    LICENSE.txt

INCLUDEPATH += header/ lib/