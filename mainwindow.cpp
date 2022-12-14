#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <QCloseEvent>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include "HexParser.hpp"
#include "crc.hpp"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStringList checkSumTypes;
    checkSumTypes << QString("LENGTH") << QString("XOR") << QString("SUM") << QString("SUM-NEG")
                  << QString("SUM-NOT") << QString("EGTS CRC8") << QString("CRC8 POLY x^8+x^7+x^4+x^0")
                  << QString("CRCMODBUS") << QString("CRCDALLAS");
    ui->comboCheckSumType->addItems(checkSumTypes);

    QStringList commandTypes;
    commandTypes << QString("USER") << QString("NAVTELECOM NTCB") << QString("NAVTELECOM NTCB FLEX");
    ui->comboCommandType->addItems(commandTypes);

    consoleHexEx = new ConsoleWindow(this);
    connect(this, &MainWindow::signal_consoleSend, consoleHexEx, &ConsoleWindow::slot_message);
    connect(this, &MainWindow::signal_consoleReceive, consoleHexEx, &ConsoleWindow::slot_message);
    connect(this, &MainWindow::signal_consoleInfo, consoleHexEx, &ConsoleWindow::slot_message);
    connect(this, &MainWindow::signal_consoleClear, consoleHexEx, &ConsoleWindow::slot_clear);
    consoleHexEx->show();

    consoleText = new ConsoleWindow(this);
    connect(this, &MainWindow::signal_consoleSend, consoleText, &ConsoleWindow::slot_message);
    connect(this, &MainWindow::signal_consoleReceive, consoleText, &ConsoleWindow::slot_message);
    connect(this, &MainWindow::signal_consoleClear, consoleText, &ConsoleWindow::slot_clear);
    consoleText->show();

    int halfWidth = width() / 2;

    QRect screenRect = screen()->availableGeometry();

    int x = (screenRect.width() - width()) / 2;
    int y = (screenRect.height() - height() - halfWidth) / 2;
    move(x, y);

    consoleHexEx->move(x, y + height());
    consoleHexEx->resize(halfWidth, halfWidth);

    consoleText->move(x + width() / 2, y + height());
    consoleText->resize(halfWidth, halfWidth);

    connect(&serialPort, &QSerialPort::readyRead, this, &MainWindow::slot_serialPort_readyRead);
    connect(&serialPort, &QSerialPort::errorOccurred, this, &MainWindow::slot_serialPort_errorOccurred);

    connect(&timerAutoSend, &QTimer::timeout, this, &MainWindow::slot_timerAutoSend_timeout);

    propertiesDialog = nullptr;
    miniConfigDialog = nullptr;

    //Console.Error(TEXT("Load props"),

     //Console.Error(TEXT("Save props"), LoadConnPropsFromFile();

    LoadConnPropsFromFile(portProperties);
    RefreshPortInfo();

    ui->actionDisconnect->trigger();
    on_buttonSendCounterReset_clicked();
}

MainWindow::~MainWindow()
{
    delete consoleHexEx;
    delete consoleText;
    delete ui;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton result = QMessageBox::question(this, QString("Question"), QString("Do you really want to exit program?"));
    if (result == QMessageBox::Yes)
    {
        ui->actionDisconnect->trigger();
        SaveConnPropsToFile(portProperties);
        qApp->quit();
    }
    else
    {
        event->ignore();
    }
}


void MainWindow::RefreshPortInfo()
{
    ui->editPort->setText(portProperties.portName);
    ui->editBaudrate->setText(QString::number(portProperties.baudRate));

    QString modeString = QString::number(portProperties.byteSize);
    modeString.append('-');
    switch (portProperties.parity)
      {
      case QSerialPort::Parity::NoParity: modeString.append('N'); break;
      case QSerialPort::Parity::OddParity: modeString.append('O'); break;
      case QSerialPort::Parity::EvenParity: modeString.append('E'); break;
      case QSerialPort::Parity::MarkParity: modeString.append('M'); break;
      case QSerialPort::Parity::SpaceParity: modeString.append('S'); break;
      default: modeString.append('?'); break;
      }
    modeString.append('-');
    switch (portProperties.stopBits)
      {
      case QSerialPort::StopBits::OneStop: modeString.append('1'); break;
      case QSerialPort::StopBits::OneAndHalfStop: modeString.append("1.5"); break;
      case QSerialPort::StopBits::TwoStop: modeString.append('2'); break;
      default: modeString.append('?'); break;
      }
    ui->editMode->setText(modeString);
}


void MainWindow::on_buttonProperties_clicked()
{
    ui->actionProperties->trigger();
}


void MainWindow::slot_propertiesDialog_finished(int result)
{
    if (result == QDialog::Accepted)
    {
        propertiesDialog->GetProperties(portProperties);
        //Console.Error(TEXT("Save props"), SaveConnPropsToFile());
        RefreshPortInfo();

        if (serialPort.isOpen())
        {
            ui->actionDisconnect->trigger();
            ui->actionConnect->trigger();
        }
    }

    delete propertiesDialog;
    propertiesDialog = nullptr;
}

void MainWindow::on_actionProperties_triggered()
{
    propertiesDialog = new CommPropsDialog(this);
    connect(propertiesDialog, &QDialog::finished, this, &MainWindow::slot_propertiesDialog_finished);
    propertiesDialog->SetProperties(portProperties);
    propertiesDialog->open();
}


void MainWindow::on_actionS_2xxx_mini_configurator_triggered()
{
    miniConfigDialog = new MiniConfigDialog(this);
    connect(miniConfigDialog, &QDialog::finished, this, &MainWindow::slot_miniConfigDialog_finished);
    miniConfigDialog->open();
}


void MainWindow::slot_miniConfigDialog_finished(int result)
{
    delete miniConfigDialog;
    miniConfigDialog = nullptr;
}


void MainWindow::on_actionConnect_triggered()
{
    emit signal_consoleInfo(QString("Connect"));

    serialPort.setPortName(portProperties.portName);
    serialPort.setBaudRate(portProperties.baudRate);
    serialPort.setDataBits(portProperties.byteSize);
    serialPort.setParity(portProperties.parity);
    serialPort.setStopBits(portProperties.stopBits);
    serialPort.open(QIODevice::ReadWrite);

    if (serialPort.isOpen())
    {
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setEnabled(false);
        ui->actionSend->setEnabled(true);
        ui->buttonSend->setEnabled(true);
        ui->spinAutoSendPeriod->setEnabled(true);
        ui->checkAutoSend->setEnabled(true);
        ui->framePortControlLines->setEnabled(true);
        ui->buttonConnection->setText(QString("Disconnect"));
    }
}


void MainWindow::on_actionDisconnect_triggered()
{
    emit signal_consoleInfo(QString("Disconnect"));

    ui->checkAutoSend->setChecked(false);

    serialPort.close();

    ui->actionDisconnect->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionSend->setEnabled(false);
    ui->buttonSend->setEnabled(false);
    ui->spinAutoSendPeriod->setEnabled(false);
    ui->checkAutoSend->setEnabled(false);
    ui->framePortControlLines->setEnabled(false);
    ui->buttonConnection->setText(QString("Connect"));
}


void MainWindow::on_buttonConnection_clicked()
{
    if (serialPort.isOpen())
    {
        ui->actionDisconnect->trigger();
    }
    else
    {
        ui->actionConnect->trigger();
    }
}


void MainWindow::on_buttonSend_clicked()
{
    ui->actionSend->trigger();
}




void MainWindow::on_actionVersion_triggered()
{
    QMessageBox::information(this, QString("Version"), QString("Build: %1 %2").arg(__DATE__).arg(__TIME__));
}


void MainWindow::on_buttonCheckSumCalculate_clicked()
{
    QString checksumText = ui->textCheckSum->toPlainText();
    QByteArray checksumData;
    HexExParser::Decode(checksumText, checksumData);

    void const *Buffer = checksumData.constData();
    size_t Length = checksumData.length();

    unsigned int Result = 0;
    switch (ui->comboCheckSumType->currentIndex())
      {
      case 0:
        Result = Length;
        break;

      case 1:
        for (unsigned long i=0;i<Length;i++)
          Result ^= ((unsigned char *)Buffer)[i];
        break;

      case 2:
        for (unsigned long i=0;i<Length;i++)
          Result += ((unsigned char *)Buffer)[i];
        break;

      case 3:
        for (unsigned long i=0;i<Length;i++)
          Result += ((unsigned char *)Buffer)[i];
        Result = -Result;
        break;

      case 4:
        for (unsigned long i=0;i<Length;i++)
          Result += ((unsigned char *)Buffer)[i];
        Result = ~Result;
        break;

      case 5:
        Result = egts_crc8((unsigned char *)Buffer, Length);
        break;

      case 6:
        Result = 0xFF - crc8_poly91_eval(0xFF, (unsigned char *)Buffer, Length);
        break;

      case 7:
        Result = eval_crc16_modbus((unsigned char *)Buffer, Length);
        break;

      case 8:
        Result = eval_dallas_crc8(0, (unsigned char *)Buffer, Length);
        break;
      }

    QString resultText = QString::number(Result, 16);
    ui->editCheckSumResult->setText(resultText);
}


void MainWindow::on_checkAutoSend_stateChanged(int arg1)
{
    if (ui->checkAutoSend->isChecked())
    {
        ui->spinAutoSendPeriod->setEnabled(false);
        timerAutoSend.start(ui->spinAutoSendPeriod->value());
    }
    else
    {
        ui->spinAutoSendPeriod->setEnabled(true);
        timerAutoSend.stop();
    }
}



int MainWindow::LoadConnPropsFromFile(CommPropsDialog::CommProperties &portProps)
{
    QFile file;
    file.setFileName(QString(propertiesFileName));

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    QJsonObject jsonProps = jsonDoc.object();
    portProps.portName = jsonProps.value(QString("port")).toString();
    portProps.baudRate = jsonProps.value(QString("baudrate")).toInt();
    portProps.byteSize = QSerialPort::DataBits(jsonProps.value(QString("bytesize")).toInt());
    portProps.parity = QSerialPort::Parity(jsonProps.value(QString("parity")).toInt());
    portProps.stopBits = QSerialPort::StopBits(jsonProps.value(QString("stopbits")).toInt());

    return (0);
}

int MainWindow::SaveConnPropsToFile(CommPropsDialog::CommProperties const &portProps)
{
    QJsonObject jsonProps;
    jsonProps.insert(QString("port"), QJsonValue(portProps.portName));
    jsonProps.insert(QString("baudrate"), QJsonValue(portProps.baudRate));
    jsonProps.insert(QString("bytesize"), QJsonValue(portProps.byteSize));
    jsonProps.insert(QString("parity"), QJsonValue(portProps.parity));
    jsonProps.insert(QString("stopbits"), QJsonValue(portProps.stopBits));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonProps);
    QByteArray jsonData = jsonDoc.toJson();

    QFile file;
    file.setFileName(QString(propertiesFileName));

    file.open(QIODevice::WriteOnly | QIODevice::Text);

    file.write(jsonData);
    file.close();

    return (0);
}


void MainWindow::on_actionSend_triggered()
{
    QString commandText = ui->textCommand->toPlainText();
    QByteArray commandData;
    HexExParser::Decode(commandText, commandData);

    switch (ui->comboCommandType->currentIndex())
      {
      default:
        serialPort.write(commandData);

        emit signal_consoleInfo(QString("Send"));

        //Console.Error(TEXT("Send"), CommPort.Send((unsigned char *)Buffer, Length));
        break;

      case 1:
        //NTCBCommand.BeginWrite();
        //NTCBCommand.Write(Buffer, Length);
        //NTCBCommand.Complete(1, 0);
        //Console.Error(TEXT("Send"), CommPort.Send((unsigned char *)NTCBCommand.Data(), NTCBCommand.Length()));

        emit signal_consoleInfo(QString("Send"));
        break;

      case 2:
        break;
      }

    ++sendCounter;
    ui->editSendCounter->setText(QString::number(sendCounter));
}


void MainWindow::on_buttonSendCounterReset_clicked()
{
    sendCounter = 0;
    ui->editSendCounter->setText(QString::number(sendCounter));
}


void MainWindow::slot_serialPort_readyRead()
{
    QByteArray dataBytes = serialPort.readAll();
    QString dataString;
    HexExParser::Encode(dataBytes, dataString);

    emit signal_consoleInfo(QString("Receive"));
    emit signal_consoleReceive(dataString);
}


void MainWindow::slot_serialPort_errorOccurred(QSerialPort::SerialPortError error)
{
    if (serialPort.isOpen())
    {
        emit signal_consoleInfo(QString::number(error));
    }
}


void MainWindow::slot_timerAutoSend_timeout()
{
    ui->actionSend->trigger();
}


void MainWindow::on_actionClear_triggered()
{
    emit signal_consoleClear();
}

