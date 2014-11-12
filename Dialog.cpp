#include "Dialog.h"
#include "ui_Dialog.h"
#include "compression.h"
#include "decompression.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    connect(ui->compression_button, SIGNAL(clicked()), this, SLOT(showCompressDialog()));
    connect(ui->decompression_button, SIGNAL(clicked()), this, SLOT(showDecompressDialog()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::showCompressDialog()
{
    QDialog parent;
    Compression * co;
    co = new Compression(&parent);
    co->setFixedSize(co->size());
    co->setWindowTitle("Кодирование");
    co->exec();
}

void Dialog::showDecompressDialog()
{
    QDialog parent;
    Decompression * de;
    de = new Decompression(&parent);
    de->setFixedSize(de->size());
    de->setWindowTitle("Декодирование");
    de->exec();
}
