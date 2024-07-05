#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QHostAddress>
#include "ngx_c_crc32.h"
#include "network_structures.h"

int g_iLenPkgHeader = sizeof(COMM_PKG_HEADER);


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &Widget::onConnected);
//    connect(socket, &QTcpSocket::readyRead, this, &Widget::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of
            (&QAbstractSocket::error), this, &Widget::onError);

    connect(ui->connect_Button, &QPushButton::clicked, this, &Widget::on_connect_Button_clicked);
    connect(ui->sendButton, &QPushButton::clicked, this, &Widget::on_sendButton_clicked);
    connect(ui->recvButton, &QPushButton::clicked, this, &Widget::on_recvButton_clicked);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_connect_Button_clicked()
{
    if (socket->state() == QAbstractSocket::ConnectingState) {
        // 如果当前正在连接中，可以考虑取消连接操作或者等待连接完成
        return;
    }
    if (socket->state() == QAbstractSocket::ConnectedState) {
        // 如果已经连接，可以直接返回或者根据需求断开连接
        return;
    }

    // 其他情况下，尝试连接到服务器
    socket->connectToHost(QHostAddress::LocalHost, 80);
    if (!socket->waitForConnected(5000)) {
        QMessageBox::critical(this, tr("Error"), tr("Connection failed!"));
    }
}


void Widget::on_sendButton_clicked()
{
    CCRC32 *p_crc32 = CCRC32::GetInstance();
    char *p_sendbuf = new char[g_iLenPkgHeader + sizeof(STRUCT_REGISTER)];
    LPCOMM_PKG_HEADER pinfohead = (LPCOMM_PKG_HEADER)p_sendbuf;
    pinfohead->msgCode = qToBigEndian<quint16>(5);
    pinfohead->pkgLen = qToBigEndian<quint16>(g_iLenPkgHeader + sizeof(STRUCT_REGISTER));


    LPSTRUCT_REGISTER pstruc_sendstruc = (LPSTRUCT_REGISTER)(p_sendbuf + g_iLenPkgHeader);
    pstruc_sendstruc->iType = qToBigEndian<quint32>(100);

    strcpy(pstruc_sendstruc->username, "1234");
    strcpy(pstruc_sendstruc->password, "beiyue99");

    pinfohead->crc32 = p_crc32->Get_CRC((unsigned char *)pstruc_sendstruc, sizeof(STRUCT_REGISTER));
    pinfohead->crc32 = qToBigEndian<quint32>(pinfohead->crc32);


    if (SendData(socket, p_sendbuf, g_iLenPkgHeader + sizeof(STRUCT_REGISTER)) == -1) {
        QMessageBox::critical(this, tr("Error"), tr("SendData() failed!"));
        delete[] p_sendbuf;
        return;
    }

    delete[] p_sendbuf;
    QMessageBox::information(this, tr("Success"), tr("successfully!"));
}



int Widget::RecvCommonData(QTcpSocket *socket, char *precvBuffer, int bufferSize)
{
    int bytesReceived = socket->read(precvBuffer, bufferSize);
    if (bytesReceived == -1) {
        return -1;
    }
    return bytesReceived;
}


void Widget::on_recvButton_clicked()
{
    char recvBuffer[100000] = {0};
    int rec = RecvCommonData(socket, recvBuffer, sizeof(recvBuffer));

    if (rec == -1) {
        QMessageBox::critical(this, tr("Error"), tr("RecvCommonData() failed!"));
    } else {
        QString message = tr("Received data length: %1 bytes").arg(rec);
        QMessageBox::information(this, tr("Received Data"), message);
    }
}






void Widget::onConnected()
{
    QMessageBox::information(this, tr("Success"), tr("Connected successfully!"));
}

//void Widget::onReadyRead()
//{
//    QByteArray data = socket->readAll();
//    int dataSize = data.size();
//    QString message = tr("Received %1 bytes of data").arg(dataSize);
//    QMessageBox::information(this, tr("Received Data"), message);
//}


void Widget::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QMessageBox::critical(this, tr("Error"), socket->errorString());
}

int Widget::SendData(QTcpSocket *socket, const char *p_sendbuf, int ibuflen)
{
    int bytesSent = socket->write(p_sendbuf, ibuflen);
    if (bytesSent == -1) {
        return -1;
    }
    socket->waitForBytesWritten();
    return bytesSent;
}


void Widget::on_sendheartButton_clicked()
{

    char *p_sendbuf = new char[g_iLenPkgHeader]; // 没有包体

    LPCOMM_PKG_HEADER pinfohead = (LPCOMM_PKG_HEADER)p_sendbuf;
    pinfohead->msgCode = 0;
    pinfohead->msgCode = htons(pinfohead->msgCode);
    pinfohead->pkgLen = htons(g_iLenPkgHeader);

    pinfohead->crc32 = 0;
    pinfohead->crc32 = htonl(pinfohead->crc32);

    if (SendData(socket, p_sendbuf, g_iLenPkgHeader) == -1) {
        QMessageBox::critical(this, tr("Error"), tr("SendData() failed"));
        delete[] p_sendbuf;
        return;
    }

    delete[] p_sendbuf;
    QMessageBox::information(this, tr("Success"), tr("Successfully sent heartbeat packet!"));

    char recvBuffer[100000] = {0};
    int rec = RecvCommonData(socket, recvBuffer, sizeof(recvBuffer));
    if (rec == -1) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to receive response"));
        return;
    }
    QMessageBox::information(this, tr("Success"), tr("Received response for heartbeat packet!"));
}




void Widget::on_floodButton_clicked()
{


    char *p_sendbuf = new char[g_iLenPkgHeader]; // 没有包体

    LPCOMM_PKG_HEADER pinfohead = (LPCOMM_PKG_HEADER)p_sendbuf;
    pinfohead->msgCode = 0;
    pinfohead->msgCode = htons(pinfohead->msgCode);
    pinfohead->pkgLen = htons(g_iLenPkgHeader);

    pinfohead->crc32 = 0;
    pinfohead->crc32 = htonl(pinfohead->crc32);

    for (int i = 0; i < 12; i++) {
        if (SendData(socket, p_sendbuf, g_iLenPkgHeader) == -1) {
            QMessageBox::critical(this, tr("Error"), tr("SendData() failed"));
            delete[] p_sendbuf;
            return;
        }
    }

    delete[] p_sendbuf;
    QMessageBox::information(this, tr("Success"), tr("Sent 12 data packets!"));
}




