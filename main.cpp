#include <QCoreApplication>
#include "clogger.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    CLogger cl;
    QObject::connect(&cl,SIGNAL(quit()),&a,SLOT(quit()));

    return a.exec();
}
