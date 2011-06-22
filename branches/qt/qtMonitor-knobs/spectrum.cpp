/* 
 * File:   spectrum.cpp
 * Author: john
 * 
 * Created on 05 August 2010, 16:12
 */

#include "spectrum.h"

spectrum::spectrum() {
}

spectrum::spectrum(QWidget*& widget) {
    QFrame::setParent(widget);
    sampleRate=96000;
    spectrumHigh=-40;
    spectrumLow=-160;
    filterLow=-3450;
    filterHigh=-150;
}

spectrum::~spectrum() {
}

void spectrum::initialize() {
    QFrame::setVisible(true);
}

void spectrum::setObjectName(QString name) {
    QFrame::setObjectName(name);
}

void spectrum::setGeometry(QRect rect) {
    QFrame::setGeometry(rect);
}

void spectrum::mousePressEvent(QMouseEvent* event) {

    //qDebug() << "mousePressEvent " << event->pos().x();

    startX=lastX=event->pos().x();
    moved=0;
}

void spectrum::mouseMoveEvent(QMouseEvent* event){
    int move=lastX - event->pos().x();
    lastX=event->pos().x();
    //qDebug() << "mouseMoveEvent " << event->pos().x() << " move:" << move;

    emit frequencyMoved(move);

}

void spectrum::mouseReleaseEvent(QMouseEvent* event) {
    int move=lastX - event->pos().x();
    lastX=event->pos().x();
    //qDebug() << "mouseReleaseEvent " << event->pos().x() << " move:" << move;

    if(moved) {
        emit frequencyMoved(move);
    } else {

    }
}

void spectrum::wheelEvent(QWheelEvent *event) {
    emit frequencyMoved(event->delta()/8/15);
}

void spectrum::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    painter.fillRect(0, 0, WIDTH, HEIGHT, Qt::black);

    // plot the filter
    int filterLeft = (filterLow - (-sampleRate / 2)) * WIDTH / sampleRate;
    int filterRight = (filterHigh - (-sampleRate / 2)) * WIDTH / sampleRate;
    painter.setBrush(Qt::SolidPattern);
    painter.fillRect(filterLeft,0,filterRight-filterLeft,HEIGHT,Qt::gray);
    
    // plot horizontal grid
    int V = spectrumHigh - spectrumLow;
    int numSteps = V / 20;
    for (int i = 1; i < numSteps; i++) {
        int num = spectrumHigh - i * 20;
        int y = (int) floor((spectrumHigh - num) * HEIGHT / V);

        painter.setPen(QPen(Qt::yellow, 1));
        painter.drawLine(0, y, WIDTH, y);

        painter.setPen(QPen(Qt::green, 1));
        painter.drawText(3,y+2,QString::number(num));

    }

    // draw cursor
    painter.setPen(QPen(Qt::red, 1));
    painter.drawLine(WIDTH/2,0,WIDTH/2,HEIGHT);

    // plot spectrum
    painter.setPen(QPen(Qt::white, 1));
    if(plot.count()==WIDTH) {
        painter.drawPolyline(plot.constData(),WIDTH);
    }

}

void spectrum::setFrequency(long long f) {
    frequency=f;
}

void spectrum::setFilter(int low, int high) {
    filterLow=low;
    filterHigh=high;
}

void spectrum::updateSpectrum(char* buffer) {
    int i;
    
    sampleRate = atoi(&buffer[32]);
    
    for(i=0;i<WIDTH;i++) {
        samples[i] = -(buffer[i + 48] & 0xFF);
    }

    plot.clear();
    for (i = 0; i < WIDTH; i++) {

        plot << QPoint(i, (int) floor(((float) spectrumHigh - samples[i])*(float) HEIGHT / (float) (spectrumHigh - spectrumLow)));
        //plot->at(i).setX(i);
        //plot->at(i).setY((int) floor(((float) spectrumHigh - samples[i])*(float) HEIGHT / (float) (spectrumHigh - spectrumLow)));

    }

    this->repaint();
}