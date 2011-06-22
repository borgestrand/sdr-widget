/*
 * File:   Configure.cpp
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 16 August 2010, 20:03
 */

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <QSettings>
#include <QComboBox>

#include "Xvtr.h"
#include "Configure.h"
#include "Mode.h"

Configure::Configure() {
    widget.setupUi(this);

    widget.sampleRateComboBox->addItem("8000");
    widget.sampleRateComboBox->addItem("48000");
    widget.audioChannelsSpinBox->setValue(1);
    widget.hostComboBox->addItem("127.0.0.1");
    widget.hostComboBox->addItem("g0orx.homelinux.net");
    widget.spectrumHighSpinBox->setValue(-40);
    widget.spectrumLowSpinBox->setValue(-160);
    widget.waterfallHighSpinBox->setValue(-60);
    widget.waterfallLowSpinBox->setValue(-120);
    widget.fpsSpinBox->setValue(15);
    widget.encodingComboBox->addItem("aLaw");
    widget.encodingComboBox->addItem("16 bit pcm");
    widget.byteOrderComboBox->addItem("LittleEndian");
    widget.byteOrderComboBox->addItem("BigEndian");
    widget.byteOrderComboBox->setCurrentIndex(0);

    widget.nrTapsSpinBox->setValue(64);
    widget.nrDelaySpinBox->setValue(8);
    widget.nrGainSpinBox->setValue(16);
    widget.nrLeakSpinBox->setValue(10);
    widget.anfTapsSpinBox->setValue(64);
    widget.anfDelaySpinBox->setValue(8);
    widget.anfGainSpinBox->setValue(32);
    widget.anfLeakSpinBox->setValue(1);
    widget.nbThresholdSpinBox->setValue(20);
    widget.sdromThresholdSpinBox->setValue(20);

    widget.ifFrequencyLineEdit->setText("28000000");

    connect(widget.spectrumHighSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotSpectrumHighChanged(int)));
    connect(widget.spectrumLowSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotSpectrumLowChanged(int)));
    connect(widget.fpsSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotFpsChanged(int)));
    connect(widget.waterfallHighSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotWaterfallHighChanged(int)));
    connect(widget.waterfallLowSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotWaterfallLowChanged(int)));

    connect(widget.audioDeviceComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotAudioDeviceChanged(int)));
    connect(widget.audioChannelsSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotChannelsChanged(int)));
    connect(widget.byteOrderComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotByteOrderChanged(int)));
    connect(widget.sampleRateComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotSampleRateChanged(int)));

    connect(widget.hostComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotHostChanged(int)));
    connect(widget.rxSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotReceiverChanged(int)));

    connect(widget.nrTapsSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotNrTapsChanged(int)));
    connect(widget.nrDelaySpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotNrDelayChanged(int)));
    connect(widget.nrGainSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotNrGainChanged(int)));
    connect(widget.nrLeakSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotNrLeakChanged(int)));

    connect(widget.anfTapsSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotAnfTapsChanged(int)));
    connect(widget.anfDelaySpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotAnfDelayChanged(int)));
    connect(widget.anfGainSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotAnfGainChanged(int)));
    connect(widget.anfLeakSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotAnfLeakChanged(int)));

    connect(widget.nbThresholdSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotNbThresholdChanged(int)));
    connect(widget.sdromThresholdSpinBox,SIGNAL(valueChanged(int)),this,SLOT(slotSdromThresholdChanged(int)));

    connect(widget.addPushButton,SIGNAL(clicked()),this,SLOT(slotXVTRAdd()));
    connect(widget.deletePushButton,SIGNAL(clicked()),this,SLOT(slotXVTRDelete()));

}

Configure::~Configure() {
}

void Configure::initAudioDevices(Audio* audio) {
    audio->get_audio_devices(widget.audioDeviceComboBox);
}

void Configure::updateXvtrList(Xvtr* xvtr) {
    // update the list of XVTR entries
    XvtrEntry* entry;
    QStringList headings;
    QString title;
    QString minFrequency;
    QString maxFrequency;
    QString ifFrequency;

    widget.XVTRTableWidget->clear();
    widget.XVTRTableWidget->setRowCount(xvtr->count());
    widget.XVTRTableWidget->setColumnCount(4);
    headings<< "Title" << "Min Frequency" << "Max Frequency" << "IF Frequency";
    widget.XVTRTableWidget->setHorizontalHeaderLabels(headings);

    for(int i=0;i<xvtr->count();i++) {
        entry=xvtr->getXvtrAt(i);
        QTableWidgetItem *titleItem=new QTableWidgetItem(entry->getTitle());
        titleItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        widget.XVTRTableWidget->setItem(i, 0, titleItem);
        minFrequency.sprintf("%lld",entry->getMinFrequency());
        QTableWidgetItem *minFrequencyItem=new QTableWidgetItem(minFrequency);
        minFrequencyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        widget.XVTRTableWidget->setItem(i, 1, minFrequencyItem);
        maxFrequency.sprintf("%lld",entry->getMaxFrequency());
        QTableWidgetItem *maxFrequencyItem=new QTableWidgetItem(maxFrequency);
        maxFrequencyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        widget.XVTRTableWidget->setItem(i, 2, maxFrequencyItem);
        ifFrequency.sprintf("%lld",entry->getIFFrequency());
        QTableWidgetItem *ifFrequencyItem=new QTableWidgetItem(ifFrequency);
        ifFrequencyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        widget.XVTRTableWidget->setItem(i, 3, ifFrequencyItem);
    }
    widget.XVTRTableWidget->resizeColumnsToContents();

}

void Configure::connected(bool state) {
    // set configuration options enabled/disabled based on connection state
    widget.audioDeviceComboBox->setDisabled(state);
    widget.sampleRateComboBox->setDisabled(state);
    widget.audioChannelsSpinBox->setDisabled(state);
    widget.encodingComboBox->setDisabled(state);
    widget.byteOrderComboBox->setDisabled(state);

    widget.hostComboBox->setDisabled(state);
    widget.rxSpinBox->setDisabled(state);


}

void Configure::loadSettings(QSettings* settings) {
    int i;

    settings->beginGroup("Servers");
    if(settings->contains("entries")) {
        widget.hostComboBox->clear();
        int entries=settings->value("entries").toInt();
        for(i=0;i<entries;i++) {
            widget.hostComboBox->addItem(settings->value(QString::number(i)).toString());
        }
        widget.hostComboBox->setCurrentIndex(settings->value("selected").toInt());
    }
    qDebug() << "server count=" << widget.hostComboBox->count();
    qDebug() << "server selected: " << widget.hostComboBox->currentIndex();

    if(settings->contains("rx")) widget.rxSpinBox->setValue(settings->value("rx").toInt());
    settings->endGroup();
    settings->beginGroup("Display");
    if(settings->contains("spectrumHigh"))widget.spectrumHighSpinBox->setValue(settings->value("spectrumHigh").toInt());
    if(settings->contains("spectrumLow"))widget.spectrumLowSpinBox->setValue(settings->value("spectrumLow").toInt());
    if(settings->contains("fps"))widget.fpsSpinBox->setValue(settings->value("fps").toInt());
    if(settings->contains("waterfallHigh"))widget.waterfallHighSpinBox->setValue(settings->value("waterfallHigh").toInt());
    if(settings->contains("waterfallLow"))widget.waterfallLowSpinBox->setValue(settings->value("waterfallLow").toInt());
    settings->endGroup();
    settings->beginGroup("Audio");
    if(settings->contains("device")) widget.audioDeviceComboBox->setCurrentIndex(settings->value("device").toInt());
    if(settings->contains("channels"))widget.audioChannelsSpinBox->setValue(settings->value("channels").toInt());
    if(settings->contains("samplerate")) widget.sampleRateComboBox->setCurrentIndex(settings->value("samplerate").toInt());
    if(settings->contains("byteorder")) widget.byteOrderComboBox->setCurrentIndex(settings->value("byteorder").toInt());
    settings->endGroup();
    settings->beginGroup("NR");
    if(settings->contains("taps")) widget.nrGainSpinBox->setValue(settings->value("taps").toInt());
    if(settings->contains("delay"))widget.nrDelaySpinBox->setValue(settings->value("delay").toInt());
    if(settings->contains("gain")) widget.nrGainSpinBox->setValue(settings->value("gain").toInt());
    if(settings->contains("leak")) widget.nrLeakSpinBox->setValue(settings->value("leak").toInt());
    settings->endGroup();
    settings->beginGroup("ANF");
    if(settings->contains("taps")) widget.anfGainSpinBox->setValue(settings->value("taps").toInt());
    if(settings->contains("delay"))widget.anfDelaySpinBox->setValue(settings->value("delay").toInt());
    if(settings->contains("gain")) widget.anfGainSpinBox->setValue(settings->value("gain").toInt());
    if(settings->contains("leak")) widget.anfLeakSpinBox->setValue(settings->value("leak").toInt());
    settings->endGroup();
    settings->beginGroup("NB");
    if(settings->contains("threshold")) widget.nbThresholdSpinBox->setValue(settings->value("threshold").toInt());
    settings->endGroup();
    settings->beginGroup("SDROM");
    if(settings->contains("threshold")) widget.nbThresholdSpinBox->setValue(settings->value("threshold").toInt());
    settings->endGroup();
}

void Configure::saveSettings(QSettings* settings) {
    int i;
    settings->beginGroup("Servers");
    qDebug() << "server count=" << widget.hostComboBox->count();
    settings->setValue("entries",widget.hostComboBox->count());
    for(i=0;i<widget.hostComboBox->count();i++) {
        qDebug() << "server: " << widget.hostComboBox->itemText(i);
        settings->setValue(QString::number(i),widget.hostComboBox->itemText(i));
    }
    settings->setValue("selected",widget.hostComboBox->currentIndex());
    qDebug() << "server selected: " << widget.hostComboBox->currentIndex();
    settings->setValue("rx",widget.rxSpinBox->value());
    settings->endGroup();
    settings->beginGroup("Display");
    settings->setValue("spectrumHigh",widget.spectrumHighSpinBox->value());
    settings->setValue("spectrumLow",widget.spectrumLowSpinBox->value());
    settings->setValue("fps",widget.fpsSpinBox->value());
    settings->setValue("waterfallHigh",widget.waterfallHighSpinBox->value());
    settings->setValue("waterfallLow",widget.waterfallLowSpinBox->value());
    settings->endGroup();
    settings->beginGroup("Audio");
    settings->setValue("device",widget.audioDeviceComboBox->currentIndex());
    settings->setValue("channels",widget.audioChannelsSpinBox->value());
    settings->setValue("samplerate",widget.sampleRateComboBox->currentIndex());
    settings->setValue("byteorder",widget.byteOrderComboBox->currentIndex());
    settings->endGroup();
    settings->beginGroup("NR");
    settings->setValue("taps",widget.nrTapsSpinBox->value());
    settings->setValue("delay",widget.nrDelaySpinBox->value());
    settings->setValue("gain",widget.nrGainSpinBox->value());
    settings->setValue("leak",widget.nrLeakSpinBox->value());
    settings->endGroup();
    settings->beginGroup("ANF");
    settings->setValue("taps",widget.anfTapsSpinBox->value());
    settings->setValue("delay",widget.anfDelaySpinBox->value());
    settings->setValue("gain",widget.anfGainSpinBox->value());
    settings->setValue("leak",widget.anfLeakSpinBox->value());
    settings->endGroup();
    settings->beginGroup("NB");
    settings->setValue("threshold",widget.nbThresholdSpinBox->value());
    settings->endGroup();
    settings->beginGroup("SDROM");
    settings->setValue("threshold",widget.nbThresholdSpinBox->value());
    settings->endGroup();
}

void Configure::slotHostChanged(int selection) {
    emit hostChanged(widget.hostComboBox->currentText());
}

void Configure::slotReceiverChanged(int receiver) {
    emit receiverChanged(receiver);
}

void Configure::slotSpectrumHighChanged(int high) {
    emit spectrumHighChanged(high);
}

void Configure::slotSpectrumLowChanged(int low) {
    emit spectrumLowChanged(low);
}

void Configure::slotFpsChanged(int fps) {
    emit fpsChanged(fps);
}

void Configure::slotWaterfallHighChanged(int high) {
    emit waterfallHighChanged(high);
}

void Configure::slotWaterfallLowChanged(int low) {
    emit waterfallLowChanged(low);
}

void Configure::slotAudioDeviceChanged(int selection) {
    emit audioDeviceChanged(widget.audioDeviceComboBox->itemData(selection).value<QAudioDeviceInfo >(),
                            widget.sampleRateComboBox->currentText().toInt(),
                            widget.audioChannelsSpinBox->value(),
                            widget.byteOrderComboBox->currentText()=="LittleEndian"?QAudioFormat::LittleEndian:QAudioFormat::BigEndian
                            );
}

void Configure::slotSampleRateChanged(int selection) {
    emit audioDeviceChanged(widget.audioDeviceComboBox->itemData(widget.audioDeviceComboBox->currentIndex()).value<QAudioDeviceInfo >(),
                            widget.sampleRateComboBox->currentText().toInt(),
                            widget.audioChannelsSpinBox->value(),
                            widget.byteOrderComboBox->currentText()=="LittleEndian"?QAudioFormat::LittleEndian:QAudioFormat::BigEndian
                            );
}

void Configure::slotChannelsChanged(int channels) {
    emit audioDeviceChanged(widget.audioDeviceComboBox->itemData(widget.audioDeviceComboBox->currentIndex()).value<QAudioDeviceInfo >(),
                            widget.sampleRateComboBox->currentText().toInt(),
                            widget.audioChannelsSpinBox->value(),
                            widget.byteOrderComboBox->currentText()=="LittleEndian"?QAudioFormat::LittleEndian:QAudioFormat::BigEndian
                            );
}

void Configure::slotByteOrderChanged(int selection) {
    emit audioDeviceChanged(widget.audioDeviceComboBox->itemData(widget.audioDeviceComboBox->currentIndex()).value<QAudioDeviceInfo >(),
                            widget.sampleRateComboBox->currentText().toInt(),
                            widget.audioChannelsSpinBox->value(),
                            widget.byteOrderComboBox->currentText()=="LittleEndian"?QAudioFormat::LittleEndian:QAudioFormat::BigEndian
                            );
}

void Configure::slotNrTapsChanged(int taps) {
    emit nrValuesChanged(widget.nrTapsSpinBox->value(),widget.nrDelaySpinBox->value(),(double)widget.nrGainSpinBox->value()*0.00001,(double)widget.nrLeakSpinBox->value()*0.0000001);
}

void Configure::slotNrDelayChanged(int delay) {
    emit nrValuesChanged(widget.nrTapsSpinBox->value(),widget.nrDelaySpinBox->value(),(double)widget.nrGainSpinBox->value()*0.00001,(double)widget.nrLeakSpinBox->value()*0.0000001);
}

void Configure::slotNrGainChanged(int gain) {
    emit nrValuesChanged(widget.nrTapsSpinBox->value(),widget.nrDelaySpinBox->value(),(double)widget.nrGainSpinBox->value()*0.00001,(double)widget.nrLeakSpinBox->value()*0.0000001);
}

void Configure::slotNrLeakChanged(int leak) {
    emit nrValuesChanged(widget.nrTapsSpinBox->value(),widget.nrDelaySpinBox->value(),(double)widget.nrGainSpinBox->value()*0.00001,(double)widget.nrLeakSpinBox->value()*0.0000001);
}

void Configure::slotAnfTapsChanged(int taps) {
    emit anfValuesChanged(widget.anfTapsSpinBox->value(),widget.anfDelaySpinBox->value(),(double)widget.anfGainSpinBox->value()*0.00001,(double)widget.anfLeakSpinBox->value()*0.0000001);
}

void Configure::slotAnfDelayChanged(int delay) {
    emit anfValuesChanged(widget.anfTapsSpinBox->value(),widget.anfDelaySpinBox->value(),(double)widget.anfGainSpinBox->value()*0.00001,(double)widget.anfLeakSpinBox->value()*0.0000001);
}

void Configure::slotAnfGainChanged(int gain) {
    emit anfValuesChanged(widget.anfTapsSpinBox->value(),widget.anfDelaySpinBox->value(),(double)widget.anfGainSpinBox->value()*0.00001,(double)widget.anfLeakSpinBox->value()*0.0000001);
}

void Configure::slotAnfLeakChanged(int leak) {
    emit anfValuesChanged(widget.anfTapsSpinBox->value(),widget.anfDelaySpinBox->value(),(double)widget.anfGainSpinBox->value()*0.00001,(double)widget.anfLeakSpinBox->value()*0.0000001);
}

void Configure::slotNbThresholdChanged(int threshold) {
    emit nbThresholdChanged((double)widget.nbThresholdSpinBox->value()*0.165);
}

void Configure::slotSdromThresholdChanged(int threshold) {
    emit sdromThresholdChanged((double)widget.sdromThresholdSpinBox->value()*0.165);
}

QString Configure::getHost() {
    return widget.hostComboBox->currentText();
}

int Configure::getReceiver() {
    return widget.rxSpinBox->value();
}

int Configure::getSpectrumHigh() {
    return widget.spectrumHighSpinBox->value();
}

int Configure::getSpectrumLow() {
    return widget.spectrumLowSpinBox->value();
}

int Configure::getFps() {
    return widget.fpsSpinBox->value();
}

int Configure::getWaterfallHigh() {
    return widget.waterfallHighSpinBox->value();
}

int Configure::getWaterfallLow() {
    return widget.waterfallLowSpinBox->value();
}

QAudioFormat::Endian Configure::getByteOrder() {
    QAudioFormat::Endian order=QAudioFormat::LittleEndian;

    switch(widget.byteOrderComboBox->currentIndex()) {
    case 0:
        order=QAudioFormat::LittleEndian;
        break;
    case 1:
        order=QAudioFormat::BigEndian;
        break;

    }

    qDebug() << "getByteOrder: " << widget.byteOrderComboBox->currentIndex() << widget.byteOrderComboBox->currentText() << " order:" << order;
    return order;
}

int Configure::getChannels() {
    return widget.audioChannelsSpinBox->value();
}

int Configure::getSampleRate() {
    return widget.sampleRateComboBox->currentText().toInt();
}

void Configure::setSpectrumLow(int low) {
    widget.spectrumLowSpinBox->setValue(low);
}

void Configure::setSpectrumHigh(int high) {
    widget.spectrumHighSpinBox->setValue(high);
}

void Configure::setWaterfallLow(int low) {
    widget.waterfallLowSpinBox->setValue(low);
}

void Configure::setWaterfallHigh(int high) {
    widget.waterfallHighSpinBox->setValue(high);
}

int Configure::getNrTaps() {
    return widget.nrTapsSpinBox->value();
}

int Configure::getNrDelay(){
    return widget.nrDelaySpinBox->value();
}

double Configure::getNrGain() {
    return (double)widget.nrGainSpinBox->value()*0.00001;
}

double Configure::getNrLeak() {
    return (double)widget.nrLeakSpinBox->value()*0.0000001;
}

int Configure::getAnfTaps() {
    return widget.anfTapsSpinBox->value();
}

int Configure::getAnfDelay() {
    return widget.anfDelaySpinBox->value();
}

double Configure::getAnfGain() {
    return (double)widget.anfGainSpinBox->value()*0.00001;
}

double Configure::getAnfLeak() {
    return (double)widget.anfLeakSpinBox->value()*0.0000001;
}

double Configure::getNbThreshold() {
    return (double)widget.nbThresholdSpinBox->value()*0.165;
}

void Configure::slotXVTRAdd() {

    QString title;
    long long minFrequency;
    long long maxFrequency;
    long long ifFrequency;

    // check name is present
    title=widget.titleLineEdit->text();

    // check min frequency
    minFrequency=widget.minFrequencyLineEdit->text().toLongLong();
    maxFrequency=widget.maxFrequencyLineEdit->text().toLongLong();
    ifFrequency=widget.ifFrequencyLineEdit->text().toLongLong();

    if(title==QString("")) {
        // must have a title
        qDebug()<<"XVTR entry must hava a title";
        return;
    }

    if(minFrequency<=0LL) {
        // must not be zero or negative
        qDebug()<<"XVTR min frequency must be > 0";
        return;
    }

    if(maxFrequency<=0LL) {
        // must not be zero or negative
        qDebug()<<"XVTR max frequency must be > 0";
        return;
    }

    if(minFrequency>=maxFrequency) {
        // max must be greater than min
        qDebug()<<"XVTR min frequency must be < max frequency";
        return;
    }

    // all looks OK so save it
    emit addXVTR(title,minFrequency,maxFrequency,ifFrequency,minFrequency,MODE_USB,5);

    // update the list
}

void Configure::slotXVTRDelete() {

    int index=widget.XVTRTableWidget->currentRow();
    if(index==-1) {
        qDebug()<<"XVTR Delete but nothing selected";
        return;
    }

    qDebug()<<"Configure::slotXVTRDelete"<<index;
    emit deleteXVTR(index);
}
