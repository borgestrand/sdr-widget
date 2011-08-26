#include "smeter.h"
#include "Spectrum.h"
#include "UI.h"
sMeter::sMeter(QWidget*& widget) {
    QFrame::setParent(widget);

    sMeterMain=new Meter("Main Rx");
    sMeterSub=new Meter("Sub Rx");
    meter_dbm = -121;
    sub_meter_dbm =-121;
    subRx = FALSE;
}

sMeter::~sMeter() {

}

void sMeter::setSubRxState(bool state)
{
    subRx=state;
}

void sMeter::paintEvent(QPaintEvent*)
{
//qDebug() << "smeter.cpp - Meter value is equal to " << meter_dbm;

    // Draw the Main Rx S-Meter
    QPainter painter(this);
    QImage image=sMeterMain->getImage(meter_dbm);
    painter.drawImage(4,0,image);

    // Draw the Sub Rx S-Meter
    if(subRx) {
        image=sMeterSub->getImage(sub_meter_dbm);
        painter.drawImage(4,image.height()+1,image);
    }
}
