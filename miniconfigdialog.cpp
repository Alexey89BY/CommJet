#include "miniconfigdialog.h"
#include "ui_miniconfigdialog.h"

MiniConfigDialog::MiniConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MiniConfigDialog)
{
    ui->setupUi(this);
}

MiniConfigDialog::~MiniConfigDialog()
{
    delete ui;
}
