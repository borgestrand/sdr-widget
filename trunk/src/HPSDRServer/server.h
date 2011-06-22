#ifndef SERVER_H
#define SERVER_H

#include <QThread>
#include <QString>
#include <QList>
#include <QtNetwork>

#include "interfaces.h"
#include "metis.h"
#include "clientserver.h"
#include "receiver.h"
#include "error.h"


class Server : public QThread {
    Q_OBJECT
public:
    Server();

    void bind();

    void setDevice(QString d);
    void setInterface(QString i);
    void setMetis(QString m);

    QUdpSocket* getSocket();

    QString getDevice();
    QString getInterface();
    QString getMetis();

    QString getDevicesHTML();
    QString getInterfacesHTML();
    QString getMetisHTML();

    QString getErrorHTML();

    int getMetisCount();

    QString getInterfaceIPAddress(QString iface);

    void discover();

    void clearMetis();
    void addMetis(Metis metis);

    void start();
    void stop();

    enum STATES {STOPPED,RUNNING};

    STATES getState();

    void setSampleRate(QString s);
    void setReceivers(QString r);
    void set10MHzClock(QString c);
    void set122_88MHzClock(QString c);
    void setPreamp(QString s);
    void setRandom(QString s);
    void setDither(QString s);
    void setDuplex(QString s);
    void setClassE(QString s);
    void setLineIn(QString s);
    void setMicBoost(QString s);
    QString getSampleRate();
    QString getReceivers();
    QString get10MHzClock();
    QString get122_88MHzClock();
    QString getPreamp();
    QString getRandom();
    QString getDither();
    QString getDuplex();
    QString getClassE();
    QString getLineIn();
    QString getMicBoost();


    void sendBuffer();

    void send_metis_buffer(int ep,unsigned char* buffer);

    int getOzySoftwareVersion();
    int getMercurySoftwareVersion();
    int getPenelopeSoftwareVersion();

    int getReceiveSequenceError();
    int getReceivedFrames();

    QString getReceiversHTML();

    QString attach(int rx,Client* c);
    void setFrequency(int rx,long f);

    void playAudio(float* buffer);

    void clearError();
    void setError(QString e);

    void setMox(int state);
    int getMox();

    float getMicGain();
    void setMicGain(float gain);

    unsigned char getControlOut(int index);

public slots:
    void readyRead();

private:
    QString device;
    QString iface;
    QString metis;

    Interfaces interfaces;;

    QList<Metis> metisCards;

    QHostAddress* metisAddress;
    QUdpSocket socket;

    STATES state;

    void startMetis();
    void stopMetis();


    QString sampleRate;
    QString receivers;
    QString clock10MHz;
    QString clock122_88MHz;
    QString preamp;
    QString random;
    QString dither;
    QString duplex;
    QString classE;
    QString line_in;
    QString mic_boost;

    unsigned char output_buffer[1032];
    unsigned char metis_buffer[512];
    int metis_buffer_index;

    unsigned long send_sequence;
    int receive_sequence_error;
    int offset;

    unsigned long receive_sequence;

    unsigned char control_out[5];
    unsigned char control_in[5];
    int ptt;
    int dash;
    int dot;

    int lt2208ADCOverflow;
    int IO1;
    int IO2;
    int IO3;
    int mercury_software_version;
    int penelope_software_version;
    int ozy_software_version;
    int forwardPower;
    int alexForwardPower;
    int AIN3;
    int AIN4;
    int AIN6;

    int mox;



    float mic_gain;

    int samples;

    unsigned long rx_frame;

    void process_iq_buffer(unsigned char* buffer);

    int start_frames;

    Receiver* receiver[4];
    int current_receiver;

    ClientServer* clientServer;

    int send_rx_frequency;
    int send_tx_frequency;

    int penny_change;

    Error error;

};

#endif // SERVER_H
