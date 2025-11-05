/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QPushButton *connect_Button;
    QPushButton *sendButton;
    QPushButton *recvButton;
    QWidget *widget1;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *sendheartButton;
    QPushButton *floodButton;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(800, 600);
        widget = new QWidget(Widget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(220, 190, 281, 26));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        connect_Button = new QPushButton(widget);
        connect_Button->setObjectName(QString::fromUtf8("connect_Button"));

        horizontalLayout->addWidget(connect_Button);

        sendButton = new QPushButton(widget);
        sendButton->setObjectName(QString::fromUtf8("sendButton"));

        horizontalLayout->addWidget(sendButton);

        recvButton = new QPushButton(widget);
        recvButton->setObjectName(QString::fromUtf8("recvButton"));

        horizontalLayout->addWidget(recvButton);

        widget1 = new QWidget(Widget);
        widget1->setObjectName(QString::fromUtf8("widget1"));
        widget1->setGeometry(QRect(270, 300, 186, 26));
        horizontalLayout_2 = new QHBoxLayout(widget1);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        sendheartButton = new QPushButton(widget1);
        sendheartButton->setObjectName(QString::fromUtf8("sendheartButton"));

        horizontalLayout_2->addWidget(sendheartButton);

        floodButton = new QPushButton(widget1);
        floodButton->setObjectName(QString::fromUtf8("floodButton"));

        horizontalLayout_2->addWidget(floodButton);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        connect_Button->setText(QCoreApplication::translate("Widget", "\350\277\236\346\216\245", nullptr));
        sendButton->setText(QCoreApplication::translate("Widget", "\345\217\221\345\214\205", nullptr));
        recvButton->setText(QCoreApplication::translate("Widget", "\346\224\266\345\214\205", nullptr));
        sendheartButton->setText(QCoreApplication::translate("Widget", "\345\217\221\345\277\203\350\267\263\345\214\205", nullptr));
        floodButton->setText(QCoreApplication::translate("Widget", "flood\346\224\273\345\207\273", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
