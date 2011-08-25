#include "smeter.h"
#include "Spectrum.h"
#include "UI.h"
sMeter::sMeter(QWidget*& widget) {
    QFrame::setParent(widget);

    sMeterMain=new Meter("Main Rx");
    sMeterSub=new Meter("Sub Rx");

}

sMeter::~sMeter() {

}

void sMeter::setSubRxState(bool state)
{
    subRx=state;
}

void sMeter::paintEvent(QPaintEvent*)
{
qDebug() << "smeter.cpp - Meter value is equal to " << meter_dbm;

    // Draw the Main Rx S-Meter
    QPainter painter(this);
    QImage image=sMeterMain->getImage(meter_dbm);
    painter.drawImage(4,0,image);

    if(subRx) {
        image=sMeterSub->getImage(meter_dbm);
        painter.drawImage(4,image.height()+1,image);
    }
}
