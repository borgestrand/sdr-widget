/* 
 * File:   spectrum.cpp
 * Author: john
 * 
 * Created on 05 August 2010, 16:12
 */

#include "waterfall.h"

waterfall::waterfall() {
}

waterfall::waterfall(QWidget*& widget) {
    int x,y;

    QFrame::setParent(widget);
    image=QImage(WIDTH, HEIGHT, QImage::Format_RGB32);

    qDebug() << image.width() << ":" << image.height();
    for(x = 0; x<WIDTH; x++) {
        for(y = 0; x<HEIGHT; x++) {
            image.setPixel(x, y, 0xFF000000);
        }
    }
    waterfallHigh=-60;
    waterfallLow=-125;
}

waterfall::~waterfall() {
}

void waterfall::initialize() {
    QFrame::setVisible(true);
}

void waterfall::setObjectName(QString name) {
    QFrame::setObjectName(name);
}

void waterfall::setGeometry(QRect rect) {
    QFrame::setGeometry(rect);
}

void waterfall::mousePressEvent(QMouseEvent* event) {

    //qDebug() << "mousePressEvent " << event->pos().x();

    startX=lastX=event->pos().x();
    moved=0;
}

void waterfall::mouseMoveEvent(QMouseEvent* event){
    int move=lastX - event->pos().x();
    lastX=event->pos().x();
    //qDebug() << "mouseMoveEvent " << event->pos().x() << " move:" << move;

    emit frequencyMoved(move);

}

void waterfall::mouseReleaseEvent(QMouseEvent* event) {
    int move=lastX - event->pos().x();
    lastX=event->pos().x();
    //qDebug() << "mouseReleaseEvent " << event->pos().x() << " move:" << move;

    if(moved) {
        emit frequencyMoved(move);
    } else {

    }
}

void waterfall::wheelEvent(QWheelEvent *event) {
    emit frequencyMoved(event->delta()/8/15);
}

void waterfall::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    painter.fillRect(0, 0, 480, 180, Qt::black);

    painter.drawImage(QPoint(0,0),image);
}


void waterfall::updateWaterfall(char* buffer) {
    int x,y;
    int sample;
    // move the pixel array down
    for(y=HEIGHT-1;y>0;y--) {
        for(x=0;x<WIDTH;x++) {
            image.setPixel(x,y,image.pixel(x,y-1));
        }
    }

    // draw the new line
    for(x=0;x<WIDTH;x++) {
        sample=0-(buffer[x+48]&0xFF);
        image.setPixel(x,0,this->calculatePixel(sample));
    }

    this->repaint();
    
}

uint waterfall::calculatePixel(int sample) {
        // simple gray scale
        int v=((int)sample-waterfallLow)*255/(waterfallHigh-waterfallLow);

        if(v<0) v=0;
        if(v>255) v=255;

        int pixel=(255<<24)+(v<<16)+(v<<8)+v;
        return pixel;
    }