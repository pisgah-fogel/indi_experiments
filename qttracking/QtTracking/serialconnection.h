#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <iostream>

class SerialConnection
{
public:
    SerialConnection();
    static QList<QSerialPortInfo> getPortInfo() {
        QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
        std::cout<<"List of serial port:"<<std::endl;
        for(QList<QSerialPortInfo>::iterator it = list.begin(); it != list.end(); it++) {
            std::cout<<"- "<<it->portName().toStdString()<<it->description().toStdString()<<std::endl;
        }
        return list;
    }
    static void connect_test(QString serialPortName) {
        QSerialPort serial;
        serial.setPortName(serialPortName);
        serial.setBaudRate(57600);
        serial.setStopBits(QSerialPort::OneStop);
        serial.setParity(QSerialPort::NoParity);
        serial.setDataBits(QSerialPort::Data8);

        if (!serial.open(QIODevice::ReadWrite)) {
            std::cout<<"Error: cannot open Serial Port "<<serialPortName.toStdString()<<std::endl;
            return;
        }

        int m_waitTimeout = 30000; // 1 second
        if (!serial.waitForReadyRead(m_waitTimeout)) {
            std::cout<<"Warning: No INITIALIZED# received"<<std::endl;
        }
        else {
            QByteArray responseData = serial.readAll();
            while (serial.waitForReadyRead(10))
                responseData += serial.readAll();
            const QString response = QString::fromUtf8(responseData);
            std::cout<<"Read from serial: "<<response.toStdString()<<std::endl;
            if(response.contains("INITIALIZED#")) {
                std::cout<<"[v] INITIALIZED# received"<<std::endl;
            }
        }

        QString currentRequest = "CONNECT#";
        std::cout<<"-> Sending "<<currentRequest.toStdString()<<std::endl;
        const QByteArray requestData = currentRequest.toUtf8();
        serial.write(requestData);
        if (!serial.waitForBytesWritten(m_waitTimeout)) {
            std::cout<<"Error: Time ran out while trying to write on the device"<<std::endl;
            serial.close();
            return;
        }

        if (!serial.waitForReadyRead(m_waitTimeout)) {
            std::cout<<"Error: No handshake received"<<std::endl;
            serial.close();
            return;
        }
        QByteArray responseData = serial.readAll();
        while (serial.waitForReadyRead(10))
            responseData += serial.readAll();
        const QString response = QString::fromUtf8(responseData);
        std::cout<<"Read from serial: "<<response.toStdString()<<std::endl;
        if(response == "OK#") {
            std::cout<<"[v] Handshake received"<<std::endl;
        }

        currentRequest = "DISCONNECT#";
        std::cout<<"-> Sending "<<currentRequest.toStdString()<<std::endl;
        const QByteArray requestData2 = currentRequest.toUtf8();
        serial.write(requestData2);
        if (!serial.waitForBytesWritten(m_waitTimeout)) {
            std::cout<<"Error: Time ran out while trying to write on the device"<<std::endl;
            serial.close();
            return;
        }

        if (!serial.waitForReadyRead(m_waitTimeout)) {
            std::cout<<"Error: No handshake received"<<std::endl;
            serial.close();
            return;
        }
        responseData = serial.readAll();
        while (serial.waitForReadyRead(10))
            responseData += serial.readAll();
        const QString response2 = QString::fromUtf8(responseData);
        std::cout<<"Read from serial: "<<response2.toStdString()<<std::endl;
        if(response2 == "OK#") {
            std::cout<<"[v] Handshake received"<<std::endl;
        }

        serial.close();
    }
};

static SerialConnection serialConnection;

#endif // SERIALCONNECTION_H
