#include "Bandscope.h"
#include "ui_Bandscope.h"

#include <QDebug>

Bandscope::Bandscope(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Bandscope)
{
    qDebug() << "Bandscope";
    ui->setupUi(this);

    QObject::connect(&connection,SIGNAL(isConnected()),this,SLOT(connected()));
    QObject::connect(&connection,SIGNAL(disconnected(QString)),this,SLOT(disconnected(QString)));
    QObject::connect(&connection,SIGNAL(bandscopeBuffer(char*,char*)),this,SLOT(bandscopeBuffer(char*,char*)));

    bandscopeHigh=0;
    bandscopeLow=-140;
}

Bandscope::~Bandscope()
{
    delete ui;
}

void Bandscope::closeEvent(QCloseEvent* event) {
    connection.disconnect();
}

void Bandscope::connect(QString host) {
    connection.connect(host,BANDSCOPE_PORT);
}

void Bandscope::disconnect() {

}

void Bandscope::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    QLinearGradient gradient(0, 0, 0,height());
    gradient.setColorAt(0, Qt::black);
    gradient.setColorAt(1, Qt::gray);
    painter.setBrush(gradient);
    painter.drawRect(0, 0, width(), height());

    // plot the vertical frequency lines
    float hzPerPixel=(float)61440000/(float)width();
    long long f=0;

    for(int i=0;i<width();i++) {
        f=(long long)(hzPerPixel*(float)i);
        if(f>0) {
            if((f%10000000)<(long long)hzPerPixel) {
                painter.setOpacity(0.5);
                painter.setPen(QPen(Qt::white, 1,Qt::DotLine));
                painter.drawLine(i, 0, i, height());

                painter.setOpacity(1.0);
                painter.setPen(QPen(Qt::black, 1));
                painter.setFont(QFont("Arial", 10));
                painter.drawText(i,height(),QString::number(f/1000000));
            }
        }
    }

    // plot Spectrum
    painter.setOpacity(1.0);
    painter.setPen(QPen(Qt::yellow, 1));
    if(plot.count()==width()) {
        painter.drawPolyline(plot.constData(),plot.count());
    }

}

void Bandscope::connected() {
    QTimer::singleShot(1000/10,this,SLOT(updateBandscope()));
}

void Bandscope::disconnected(QString message) {
}

void Bandscope::updateBandscope() {
    QString command;
    command.clear(); QTextStream(&command) << "getbandscope " << width();
    connection.sendCommand(command);
}

void Bandscope::bandscopeBuffer(char* header,char* buffer) {
    int i;
    int length=atoi(&header[26]);
    if(length==width()) {
        float* samples = (float*) malloc(length * sizeof (float));
        for(i=0;i<length;i++) {
            samples[i] = -(buffer[i] & 0xFF);
        }
        //qDebug() << "updateSpectrum: create plot points";
        plot.clear();
        for (i = 0; i < length; i++) {
            plot << QPoint(i, (int) floor(((float) bandscopeHigh - samples[i])*(float) height() / (float) (bandscopeHigh - bandscopeLow)));
        }
        free(samples);
        repaint();
    }

    QTimer::singleShot(1000/10,this,SLOT(updateBandscope()));
}
