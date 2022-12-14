#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include <QMainWindow>

namespace Ui {
class ConsoleWindow;
}

class ConsoleWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConsoleWindow(QWidget *parent = nullptr);
    ~ConsoleWindow();

public slots:
    void slot_message(QString const &message);
    void slot_clear();

protected:
     void closeEvent(QCloseEvent *event);

private:
    Ui::ConsoleWindow *ui;
};

#endif // CONSOLEWINDOW_H
