#include "CSCPTest/CSCPTest.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CSCPTest w;
    w.show();
    return QApplication::exec();
}
