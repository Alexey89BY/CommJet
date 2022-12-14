#ifndef MINICONFIGDIALOG_H
#define MINICONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class MiniConfigDialog;
}

class MiniConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MiniConfigDialog(QWidget *parent = nullptr);
    ~MiniConfigDialog();

private:
    Ui::MiniConfigDialog *ui;
};

#endif // MINICONFIGDIALOG_H
