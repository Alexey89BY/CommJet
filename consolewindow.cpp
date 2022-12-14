#include "consolewindow.h"
#include "ui_consolewindow.h"
#include <QCloseEvent>


ConsoleWindow::ConsoleWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConsoleWindow)
{
    ui->setupUi(this);
}

ConsoleWindow::~ConsoleWindow()
{
    delete ui;
}


void ConsoleWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
}


void ConsoleWindow::slot_message(QString const &message)
{
    ui->plainTextEdit->appendPlainText(message);
}


void ConsoleWindow::slot_clear()
{
    ui->plainTextEdit->clear();
}
