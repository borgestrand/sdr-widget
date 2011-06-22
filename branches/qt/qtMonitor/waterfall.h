/* 
 * File:   spectrum.h
 * Author: john
 *
 * Created on 05 August 2010, 16:12
 */

#ifndef WATERFALL_H
#define	WATERFALL_H

#include <QtCore>

#include <QFrame>
#include <QPainter>
#include <QMouseEvent>

#define WIDTH 480
#define HEIGHT 90

class waterfall: public QFrame {
    Q_OBJECT
public:
    waterfall();
    waterfall(QWidget*& widget);
    virtual ~waterfall();
    void setObjectName(QString name);
    void setGeometry(QRect rect);
    void initialize();  
    void setFrequency(long long f);
    void setFilter(int low,int high);
    void updateWaterfall(char* buffer);
    

signals:
    void frequencyMoved(int step);

protected:
    void paintEvent(QPaintEvent*);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void wheelEvent(QWheelEvent *event);
    
private:
    uint calculatePixel(int sample);
    
    float samples[WIDTH];
    int   X[WIDTH];
    int   Y[WIDTH];
    int waterfallHigh;
    int waterfallLow;

    int startX;
    int lastX;
    int moved;

    QImage image;
};


#endif	/* WATERFALL_H */

