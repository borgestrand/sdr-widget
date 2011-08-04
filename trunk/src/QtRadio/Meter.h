#ifndef METER_H
#define METER_H

#include <QDebug>
#include <QImage>
#include <QPainter>

class Meter {
public:
    Meter(QString title);
    QImage getImage(int dbm);

private:
    QImage* image;
    int x,y;
    int dxmin,dymin,dxmax,dymax;
    QString strDbm;

    void calculateLine(int dbm,double minRadius,double maxRadius);
};

#endif // METER_H
