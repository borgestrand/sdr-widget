/* 
 * File:   spectrum.h
 * Author: john
 *
 * Created on 05 August 2010, 16:12
 */

#ifndef SPECTRUM_H
#define	SPECTRUM_H

#include <QtCore>

#include <QFrame>
#include <QPainter>
#include <QMouseEvent>

#define WIDTH 480
#define HEIGHT 90

class spectrum: public QFrame {
    Q_OBJECT
public:
    spectrum();
    spectrum(QWidget*& widget);
    virtual ~spectrum();
    void setObjectName(QString name);
    void setGeometry(QRect rect);
    void initialize();  
    void setFrequency(long long f);
    void setFilter(int low,int high);
    void updateSpectrum(char* buffer);

signals:
    void frequencyMoved(int step);

protected:
    void paintEvent(QPaintEvent*);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void wheelEvent(QWheelEvent *event);
    
private:
    float samples[WIDTH];
    int   X[WIDTH];
    int   Y[WIDTH];
    int spectrumHigh;
    int spectrumLow;

    int startX;
    int lastX;
    int moved;

    int sampleRate;

    int filterLow;
    int filterHigh;
    long long frequency;

    QVector <QPoint> plot;
};


#endif	/* SPECTRUM_H */

