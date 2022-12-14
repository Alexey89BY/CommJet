#ifndef COMMPROPSDIALOG_H
#define COMMPROPSDIALOG_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class CommPropsDialog;
}

class CommPropsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommPropsDialog(QWidget *parent = nullptr);
    ~CommPropsDialog();

    struct CommProperties
    {
      QString portName;
      qint32 baudRate;
      QSerialPort::DataBits byteSize;
      QSerialPort::Parity parity;
      QSerialPort::StopBits stopBits;
    };

    void GetProperties(CommProperties &properties);
    void SetProperties(CommProperties const &properties);

private slots:
    void slot_buttonSearchPorts_clicked();

private:
    Ui::CommPropsDialog *ui;
    QList<QSerialPort::DataBits> listByteSizeValues;
    QList<QSerialPort::Parity> listParityValues;
    QList<QSerialPort::StopBits> listStopBitsValues;
};

#endif // COMMPROPSDIALOG_H
