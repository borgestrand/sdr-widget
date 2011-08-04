#include <math.h>
#include "Meter.h"

#define CENTER_X 75
#define CENTER_Y 85

Meter::Meter(QString title) {
    image=new QImage(150,60,QImage::Format_ARGB32);
    image->fill(0xFFFFFFFF);

    QPainter painter(image);

    painter.setFont(QFont("Arial", 8));

    painter.setPen(QPen(Qt::red, 1));
    painter.drawArc(5, 15, 140, 140, -260*16, -60*16);

    painter.setPen(QPen(Qt::black, 1));
    painter.drawArc(5, 15, 140, 140, -212*16, -48*16);

    // put in the markers
    painter.setPen(Qt::black);
    calculateLine(-121, 60, 75); // 1
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    painter.drawText(dxmax-5, dymax,"1");
    calculateLine(-115, 65, 70); // 2
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    calculateLine(-109, 60, 75); // 3
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    painter.drawText(dxmax-5, dymax,"3");
    calculateLine(-103, 65, 70); // 4
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    calculateLine(-97, 65, 70); // 5
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    calculateLine(-91, 60, 75); // 6
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    painter.drawText(dxmax-5, dymax,"6");
    calculateLine(-85, 65, 70); // 7
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    calculateLine(-79, 65, 70); // 8
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    calculateLine(-73, 60, 75); // 9
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    painter.drawText(dxmax-5, dymax,"9");

    painter.setPen(Qt::red);
    calculateLine(-53, 60, 75); // +20
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    painter.drawText(dxmax-5, dymax,"+20");
    calculateLine(-33, 60, 75); // +40
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    painter.drawText(dxmax-5, dymax,"+40");
    calculateLine(-13, 60, 75); // +60
    painter.drawLine(dxmin, dymin, dxmax, dymax);
    painter.drawText(dxmax-5, dymax,"+60");

    painter.drawText(0,0,image->width(),image->height(),Qt::AlignBottom||Qt::AlignHCenter,title);
}

void Meter::calculateLine(int dbm, double minRadius, double maxRadius) {
    double degrees=-121-(dbm+121);
    double radians = degrees*(3.14159265358979323846/180.0);
    double sine   = sin(radians);
    double cosine = cos(radians);

    dxmin = CENTER_X + (int)(minRadius * sine);
    dymin = CENTER_Y + (int)(minRadius * cosine);

    dxmax = CENTER_X + (int)(maxRadius * sine);
    dymax = CENTER_Y + (int)(maxRadius * cosine);

}

QImage Meter::getImage(int dbm) {
    QImage qImage(*image);
    QPainter painter(&qImage);

    painter.setPen(Qt::blue);
    calculateLine(dbm,0,75);
    painter.drawLine(dxmin, dymin, dxmax, dymax);

    painter.setPen(Qt::black);
    strDbm.sprintf("%d dBm",dbm);
    painter.drawText(image->width()-80,image->height()-2,strDbm);

    return qImage;
}
