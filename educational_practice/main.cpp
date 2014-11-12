#include <QApplication>
#include "Dialog.h"
#include "compression.h"
#include "decompression.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}
