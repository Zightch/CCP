#include "CCPTest/CCPTest.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CCPTest w;
    w.show();
    return QApplication::exec();
}
