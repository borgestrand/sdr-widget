/********************************************************************************
** Form generated from reading UI file 'qtMonitor.ui'
**
** Created: Fri Aug 13 11:59:51 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTMONITOR_H
#define UI_QTMONITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDial>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>
#include "spectrum.h"
#include "waterfall.h"

QT_BEGIN_NAMESPACE

class Ui_qtMonitor
{
public:
    QWidget *centralwidget;
    QGroupBox *groupBox;
    QPushButton *band_160_pushButton;
    QPushButton *band_80_pushButton;
    QPushButton *band_60_pushButton;
    QPushButton *band_40_pushButton;
    QPushButton *band_30_pushButton;
    QPushButton *band_20_pushButton;
    QPushButton *band_17_pushButton;
    QPushButton *band_15_pushButton;
    QPushButton *band_12_pushButton;
    QPushButton *band_10_pushButton;
    QPushButton *band_6_pushButton;
    QPushButton *band_gen_pushButton;
    QGroupBox *groupBox_2;
    QPushButton *mode_lsb_pushButton;
    QPushButton *mode_usb_pushButton;
    QPushButton *mode_dsb_pushButton;
    QPushButton *mode_cwl_pushButton;
    QPushButton *mode_cwu_pushButton;
    QPushButton *mode_am_pushButton;
    QLCDNumber *vfo_lcdNumber;
    QLabel *serverLabel;
    QLineEdit *server_lineEdit;
    QLabel *label;
    QSpinBox *rx_spinBox;
    QPushButton *connect_pushButton;
    spectrum *spectrumFrame;
    waterfall *waterfallFrame;
    QComboBox *audioComboBox;
    QLabel *label_2;
    QDial *vfoDial;
    QDial *afgainDial;
    QLabel *label_3;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *qtMonitor)
    {
        if (qtMonitor->objectName().isEmpty())
            qtMonitor->setObjectName(QString::fromUtf8("qtMonitor"));
        qtMonitor->resize(717, 382);
        centralwidget = new QWidget(qtMonitor);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(20, 40, 151, 141));
        band_160_pushButton = new QPushButton(groupBox);
        band_160_pushButton->setObjectName(QString::fromUtf8("band_160_pushButton"));
        band_160_pushButton->setGeometry(QRect(0, 20, 51, 27));
        band_80_pushButton = new QPushButton(groupBox);
        band_80_pushButton->setObjectName(QString::fromUtf8("band_80_pushButton"));
        band_80_pushButton->setGeometry(QRect(50, 20, 51, 27));
        band_60_pushButton = new QPushButton(groupBox);
        band_60_pushButton->setObjectName(QString::fromUtf8("band_60_pushButton"));
        band_60_pushButton->setGeometry(QRect(100, 20, 51, 27));
        band_40_pushButton = new QPushButton(groupBox);
        band_40_pushButton->setObjectName(QString::fromUtf8("band_40_pushButton"));
        band_40_pushButton->setGeometry(QRect(0, 50, 51, 27));
        band_30_pushButton = new QPushButton(groupBox);
        band_30_pushButton->setObjectName(QString::fromUtf8("band_30_pushButton"));
        band_30_pushButton->setGeometry(QRect(50, 50, 51, 27));
        band_20_pushButton = new QPushButton(groupBox);
        band_20_pushButton->setObjectName(QString::fromUtf8("band_20_pushButton"));
        band_20_pushButton->setGeometry(QRect(100, 50, 51, 27));
        band_17_pushButton = new QPushButton(groupBox);
        band_17_pushButton->setObjectName(QString::fromUtf8("band_17_pushButton"));
        band_17_pushButton->setGeometry(QRect(0, 80, 51, 27));
        band_15_pushButton = new QPushButton(groupBox);
        band_15_pushButton->setObjectName(QString::fromUtf8("band_15_pushButton"));
        band_15_pushButton->setGeometry(QRect(50, 80, 51, 27));
        band_12_pushButton = new QPushButton(groupBox);
        band_12_pushButton->setObjectName(QString::fromUtf8("band_12_pushButton"));
        band_12_pushButton->setGeometry(QRect(100, 80, 51, 27));
        band_10_pushButton = new QPushButton(groupBox);
        band_10_pushButton->setObjectName(QString::fromUtf8("band_10_pushButton"));
        band_10_pushButton->setGeometry(QRect(0, 110, 51, 27));
        band_6_pushButton = new QPushButton(groupBox);
        band_6_pushButton->setObjectName(QString::fromUtf8("band_6_pushButton"));
        band_6_pushButton->setGeometry(QRect(50, 110, 51, 27));
        band_gen_pushButton = new QPushButton(groupBox);
        band_gen_pushButton->setObjectName(QString::fromUtf8("band_gen_pushButton"));
        band_gen_pushButton->setGeometry(QRect(100, 110, 51, 27));
        groupBox_2 = new QGroupBox(centralwidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(20, 190, 151, 81));
        mode_lsb_pushButton = new QPushButton(groupBox_2);
        mode_lsb_pushButton->setObjectName(QString::fromUtf8("mode_lsb_pushButton"));
        mode_lsb_pushButton->setGeometry(QRect(0, 20, 51, 27));
        mode_usb_pushButton = new QPushButton(groupBox_2);
        mode_usb_pushButton->setObjectName(QString::fromUtf8("mode_usb_pushButton"));
        mode_usb_pushButton->setGeometry(QRect(50, 20, 51, 27));
        mode_dsb_pushButton = new QPushButton(groupBox_2);
        mode_dsb_pushButton->setObjectName(QString::fromUtf8("mode_dsb_pushButton"));
        mode_dsb_pushButton->setGeometry(QRect(100, 20, 51, 27));
        mode_cwl_pushButton = new QPushButton(groupBox_2);
        mode_cwl_pushButton->setObjectName(QString::fromUtf8("mode_cwl_pushButton"));
        mode_cwl_pushButton->setGeometry(QRect(0, 50, 51, 27));
        mode_cwu_pushButton = new QPushButton(groupBox_2);
        mode_cwu_pushButton->setObjectName(QString::fromUtf8("mode_cwu_pushButton"));
        mode_cwu_pushButton->setGeometry(QRect(50, 50, 51, 27));
        mode_am_pushButton = new QPushButton(groupBox_2);
        mode_am_pushButton->setObjectName(QString::fromUtf8("mode_am_pushButton"));
        mode_am_pushButton->setGeometry(QRect(100, 50, 51, 27));
        vfo_lcdNumber = new QLCDNumber(centralwidget);
        vfo_lcdNumber->setObjectName(QString::fromUtf8("vfo_lcdNumber"));
        vfo_lcdNumber->setGeometry(QRect(0, 0, 211, 41));
        vfo_lcdNumber->setNumDigits(10);
        vfo_lcdNumber->setProperty("value", QVariant(7.056e+06));
        serverLabel = new QLabel(centralwidget);
        serverLabel->setObjectName(QString::fromUtf8("serverLabel"));
        serverLabel->setGeometry(QRect(400, 240, 51, 17));
        server_lineEdit = new QLineEdit(centralwidget);
        server_lineEdit->setObjectName(QString::fromUtf8("server_lineEdit"));
        server_lineEdit->setGeometry(QRect(450, 240, 113, 25));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(570, 240, 31, 20));
        rx_spinBox = new QSpinBox(centralwidget);
        rx_spinBox->setObjectName(QString::fromUtf8("rx_spinBox"));
        rx_spinBox->setGeometry(QRect(600, 240, 50, 25));
        rx_spinBox->setMaximum(3);
        connect_pushButton = new QPushButton(centralwidget);
        connect_pushButton->setObjectName(QString::fromUtf8("connect_pushButton"));
        connect_pushButton->setGeometry(QRect(500, 280, 91, 27));
        spectrumFrame = new spectrum(centralwidget);
        spectrumFrame->setObjectName(QString::fromUtf8("spectrumFrame"));
        spectrumFrame->setGeometry(QRect(220, 0, 481, 91));
        spectrumFrame->setFrameShape(QFrame::NoFrame);
        spectrumFrame->setFrameShadow(QFrame::Plain);
        waterfallFrame = new waterfall(centralwidget);
        waterfallFrame->setObjectName(QString::fromUtf8("waterfallFrame"));
        waterfallFrame->setGeometry(QRect(220, 90, 481, 91));
        waterfallFrame->setFrameShape(QFrame::StyledPanel);
        waterfallFrame->setFrameShadow(QFrame::Raised);
        audioComboBox = new QComboBox(centralwidget);
        audioComboBox->setObjectName(QString::fromUtf8("audioComboBox"));
        audioComboBox->setGeometry(QRect(450, 200, 201, 29));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(400, 210, 51, 17));
        vfoDial = new QDial(centralwidget);
        vfoDial->setObjectName(QString::fromUtf8("vfoDial"));
        vfoDial->setGeometry(QRect(170, 180, 221, 191));
        afgainDial = new QDial(centralwidget);
        afgainDial->setObjectName(QString::fromUtf8("afgainDial"));
        afgainDial->setGeometry(QRect(80, 270, 50, 64));
        afgainDial->setValue(30);
        afgainDial->setTracking(true);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(80, 330, 62, 17));
        qtMonitor->setCentralWidget(centralwidget);
        statusBar = new QStatusBar(qtMonitor);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        qtMonitor->setStatusBar(statusBar);

        retranslateUi(qtMonitor);

        QMetaObject::connectSlotsByName(qtMonitor);
    } // setupUi

    void retranslateUi(QMainWindow *qtMonitor)
    {
        qtMonitor->setWindowTitle(QApplication::translate("qtMonitor", "qtMonitor", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("qtMonitor", "Band", 0, QApplication::UnicodeUTF8));
        band_160_pushButton->setText(QApplication::translate("qtMonitor", "160", 0, QApplication::UnicodeUTF8));
        band_80_pushButton->setText(QApplication::translate("qtMonitor", "80", 0, QApplication::UnicodeUTF8));
        band_60_pushButton->setText(QApplication::translate("qtMonitor", "60", 0, QApplication::UnicodeUTF8));
        band_40_pushButton->setText(QApplication::translate("qtMonitor", "40", 0, QApplication::UnicodeUTF8));
        band_30_pushButton->setText(QApplication::translate("qtMonitor", "30", 0, QApplication::UnicodeUTF8));
        band_20_pushButton->setText(QApplication::translate("qtMonitor", "20", 0, QApplication::UnicodeUTF8));
        band_17_pushButton->setText(QApplication::translate("qtMonitor", "17", 0, QApplication::UnicodeUTF8));
        band_15_pushButton->setText(QApplication::translate("qtMonitor", "15", 0, QApplication::UnicodeUTF8));
        band_12_pushButton->setText(QApplication::translate("qtMonitor", "12", 0, QApplication::UnicodeUTF8));
        band_10_pushButton->setText(QApplication::translate("qtMonitor", "10", 0, QApplication::UnicodeUTF8));
        band_6_pushButton->setText(QApplication::translate("qtMonitor", "6", 0, QApplication::UnicodeUTF8));
        band_gen_pushButton->setText(QApplication::translate("qtMonitor", "Gen", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("qtMonitor", "Mode", 0, QApplication::UnicodeUTF8));
        mode_lsb_pushButton->setText(QApplication::translate("qtMonitor", "LSB", 0, QApplication::UnicodeUTF8));
        mode_usb_pushButton->setText(QApplication::translate("qtMonitor", "USB", 0, QApplication::UnicodeUTF8));
        mode_dsb_pushButton->setText(QApplication::translate("qtMonitor", "DSB", 0, QApplication::UnicodeUTF8));
        mode_cwl_pushButton->setText(QApplication::translate("qtMonitor", "CWL", 0, QApplication::UnicodeUTF8));
        mode_cwu_pushButton->setText(QApplication::translate("qtMonitor", "CWU", 0, QApplication::UnicodeUTF8));
        mode_am_pushButton->setText(QApplication::translate("qtMonitor", "AM", 0, QApplication::UnicodeUTF8));
        serverLabel->setText(QApplication::translate("qtMonitor", "Server:", 0, QApplication::UnicodeUTF8));
        server_lineEdit->setText(QApplication::translate("qtMonitor", "127.0.0.1", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("qtMonitor", "Rx:", 0, QApplication::UnicodeUTF8));
        connect_pushButton->setText(QApplication::translate("qtMonitor", "Connect", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("qtMonitor", "Audio:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("qtMonitor", "AF Gain", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class qtMonitor: public Ui_qtMonitor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTMONITOR_H
