#include "CFUPTest/CFUPTest.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CFUPTest w;
    w.show();
    return QApplication::exec();
}
