#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include "consolewindow.h"
#include "commpropsdialog.h"
#include "miniconfigdialog.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void signal_consoleReceive(QString const &message);
    void signal_consoleSend(QString const &message);
    void signal_consoleInfo(QString const &message); //, QColor color = QColor());
    void signal_consoleClear();

protected:
     void closeEvent(QCloseEvent *event);

private slots:
    void on_buttonProperties_clicked();
    void on_actionProperties_triggered();
    void on_actionS_2xxx_mini_configurator_triggered();

    void slot_serialPort_readyRead();
    void slot_serialPort_errorOccurred(QSerialPort::SerialPortError error);

    void slot_propertiesDialog_finished(int result);
    void slot_miniConfigDialog_finished(int result);
    void slot_timerAutoSend_timeout();

    void on_actionDisconnect_triggered();

    void on_buttonConnection_clicked();

    void on_buttonSend_clicked();

    void on_actionConnect_triggered();

    void on_actionVersion_triggered();

    void on_buttonCheckSumCalculate_clicked();

    void on_checkAutoSend_stateChanged(int arg1);

    void on_actionSend_triggered();

    void on_buttonSendCounterReset_clicked();

    void on_actionClear_triggered();

private:
    const char* propertiesFileName = "settings.bin";

    Ui::MainWindow *ui;
    ConsoleWindow *consoleHexEx;
    ConsoleWindow *consoleText;
    CommPropsDialog *propertiesDialog;
    MiniConfigDialog *miniConfigDialog;
    QSerialPort serialPort;
    QTimer timerAutoSend;
    CommPropsDialog::CommProperties portProperties;
    int sendCounter;


    int LoadConnPropsFromFile(CommPropsDialog::CommProperties &portProps);
    int SaveConnPropsToFile(CommPropsDialog::CommProperties const &portProps);
    void RefreshPortInfo();
};
#endif // MAINWINDOW_H
