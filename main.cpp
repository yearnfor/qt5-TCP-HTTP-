#include <QCoreApplication>
#include "mainservice.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MainService service;
    service.initial();
    service.start();
    return a.exec();
}
