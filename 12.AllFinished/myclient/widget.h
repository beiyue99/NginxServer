#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include "network_structures.h"
#include <QtEndian>
#include <winsock2.h>
#include<QTextEdit>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onConnected();
//    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);


    void on_connect_Button_clicked();

    void on_sendButton_clicked();

    void on_recvButton_clicked();

    void on_sendheartButton_clicked();

    void on_floodButton_clicked();



private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    int SendData(QTcpSocket *socket, const char *p_sendbuf, int ibuflen);
    int RecvCommonData(QTcpSocket *socket, char *precvBuffer, int bufferSize);
};
#endif // WIDGET_H
