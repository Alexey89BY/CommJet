#include "commpropsdialog.h"
#include "ui_commpropsdialog.h"
#include <QSerialPortInfo>


CommPropsDialog::CommPropsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommPropsDialog)
{
    ui->setupUi(this);

    static QList<qint32> listBaudRates = QSerialPortInfo::standardBaudRates();
    foreach (auto baudRate, listBaudRates)
    {
        ui->comboBaudRate->addItem(QString::number(baudRate));
    }
    ui->comboBaudRate->setValidator(new QIntValidator(100, 16000000));

    QStringList listByteSize;
    listByteSize << QString("5") << QString("6") << QString("7") << QString("8");
    listByteSizeValues << QSerialPort::DataBits::Data5 << QSerialPort::DataBits::Data6 << QSerialPort::DataBits::Data7 << QSerialPort::DataBits::Data8;
    ui->comboByteSize->addItems(listByteSize);

    QStringList listParity;
    listParity << QString("No parity") << QString("Odd") << QString("Even") << QString("Mark") << QString("Space");
    listParityValues << QSerialPort::Parity::NoParity << QSerialPort::Parity::OddParity << QSerialPort::Parity::EvenParity << QSerialPort::Parity::MarkParity << QSerialPort::Parity::SpaceParity;
    ui->comboParity->addItems(listParity);

    QStringList listStopBitsTypes;
    listStopBitsTypes << QString("1") << QString("1.5") << QString("2");
    listStopBitsValues << QSerialPort::StopBits::OneStop << QSerialPort::StopBits::OneAndHalfStop << QSerialPort::StopBits::TwoStop;
    ui->comboStopBits->addItems(listStopBitsTypes);

    connect(ui->buttonSearchPorts, &QPushButton::clicked, this, &CommPropsDialog::slot_buttonSearchPorts_clicked);
    slot_buttonSearchPorts_clicked();
    ui->comboPort->setCurrentIndex(0);
}


CommPropsDialog::~CommPropsDialog()
{
    delete ui;
}


void CommPropsDialog::GetProperties(CommProperties &properties)
{
    properties.portName = ui->comboPort->currentText();
    properties.baudRate = ui->comboBaudRate->currentText().toInt();
    properties.byteSize = listByteSizeValues.value(ui->comboByteSize->currentIndex(), QSerialPort::DataBits::UnknownDataBits);
    properties.parity = listParityValues.value(ui->comboParity->currentIndex(), QSerialPort::Parity::UnknownParity);
    properties.stopBits = listStopBitsValues.value(ui->comboStopBits->currentIndex(), QSerialPort::StopBits::UnknownStopBits);
}


void CommPropsDialog::SetProperties(CommProperties const &properties)
{
    ui->comboPort->setCurrentText(properties.portName);
    ui->comboBaudRate->setCurrentText(QString::number(properties.baudRate));
    ui->comboByteSize->setCurrentIndex(listByteSizeValues.indexOf(properties.byteSize));
    ui->comboParity->setCurrentIndex(listParityValues.indexOf(properties.parity));
    ui->comboStopBits->setCurrentIndex(listStopBitsValues.indexOf(properties.stopBits));
}


void CommPropsDialog::slot_buttonSearchPorts_clicked()
{
    QString selectedPort = ui->comboPort->currentText();

    ui->comboPort->clear();
    QList<QSerialPortInfo> listPorts = QSerialPortInfo::availablePorts();
    foreach (auto &portInfo, listPorts)
    {
        ui->comboPort->addItem(portInfo.portName());
    }

    ui->comboPort->setCurrentText(selectedPort);
}

