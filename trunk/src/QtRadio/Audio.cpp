/* 
 * File:   Audio.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 16 August 2010, 11:19
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

#include "Audio.h"
#include "codec2.h"

Audio::Audio() {
    audio_output=NULL;
    sampleRate=8000;
    audio_encoding = 0;
    audio_channels=1;
    audio_byte_order=QAudioFormat::LittleEndian;

    qDebug() << "Audio: LittleEndian=" << QAudioFormat::LittleEndian << " BigEndian=" << QAudioFormat::BigEndian;

    audio_format.setSampleType(QAudioFormat::SignedInt);
    audio_format.setFrequency(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannels(audio_channels);
    audio_format.setSampleSize(16);
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(audio_byte_order);
    codec2 = codec2_create();

}

Audio::~Audio() {
    codec2_destroy(codec2);
}


void Audio::initialize_audio(int buffer_size) {
    qDebug() << "initialize_audio " << buffer_size;

    if ( (buffer_size*4) < CODEC2_SAMPLES_PER_FRAME*8)
        decoded_buffer.resize(CODEC2_SAMPLES_PER_FRAME*8); // To cater to 8 frames of codec2
    else decoded_buffer.resize(buffer_size*4);  // To cater to 2 channels and 16 bits

    init_decodetable();
}

void Audio::get_audio_devices(QComboBox* comboBox) {

    QList<QAudioDeviceInfo> devices=QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    QAudioDeviceInfo device_info;

    qDebug() << "Audio::get_audio_devices";
    for(int i=0;i<devices.length();i++) {
        device_info=devices.at(i);
        qDebug() << "Audio::get_audio_devices: " << device_info.deviceName();

        qDebug() << "    Codecs:";
        QStringList codecs=device_info.supportedCodecs();
        for(int j=0;j<codecs.size();j++) {
            qDebug() << "        " << codecs.at(j).toLocal8Bit().constData();
        }

        qDebug() << "    Byte Orders";
        QList<QAudioFormat::Endian> byteOrders=device_info.supportedByteOrders();
        for(int j=0;j<byteOrders.size();j++) {
            qDebug() << "        " << (byteOrders.at(j)==QAudioFormat::BigEndian?"BigEndian":"LittleEndian");
        }

        qDebug() << "    Sample Type";
        QList<QAudioFormat::SampleType> sampleTypes=device_info.supportedSampleTypes();
        for(int j=0;j<sampleTypes.size();j++) {
            if(sampleTypes.at(j)==QAudioFormat::Unknown) {
                qDebug() << "        Unknown";
            } else if(sampleTypes.at(j)==QAudioFormat::SignedInt) {
                qDebug() << "        SignedInt";
            } else if(sampleTypes.at(j)==QAudioFormat::UnSignedInt) {
                qDebug() << "        UnSignedInt";
            } else if(sampleTypes.at(j)==QAudioFormat::Float) {
                qDebug() << "        Float";
            }
        }

        qDebug() << "    Sample Rates";
        QList<int> sampleRates=device_info.supportedFrequencies();
        for(int j=0;j<sampleRates.size();j++) {
            qDebug() << "        " << sampleRates.at(j);
        }

        qDebug() << "    Sample Sizes";
        QList<int> sampleSizes=device_info.supportedSampleSizes();
        for(int j=0;j<sampleSizes.size();j++) {
            qDebug() << "        " << sampleSizes.at(j);
        }

        qDebug() << "    Channels";
        QList<int> channels=device_info.supportedChannels();
        for(int j=0;j<channels.size();j++) {
            qDebug() << "        " << channels.at(j);
        }

        comboBox->addItem(device_info.deviceName(),qVariantFromValue(device_info));
        if(i==0) {
            audio_device=device_info;
        }
        i++;
    }

    qDebug() << "Audio::get_audio_devices: default is " << audio_device.deviceName();

    audio_output = new QAudioOutput(audio_device, audio_format, this);
    connect(audio_output,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));

    qDebug() << "QAudioOutput: error=" << audio_output->error() << " state=" << audio_output->state();

  //  audio_output->setBufferSize(1024*48);
    audio_out = audio_output->start();

    if(audio_output->error()!=0) {
        qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << audio_output->state();

        qDebug() << "Format:";
        qDebug() << "    sample rate: " << audio_format.frequency();
        qDebug() << "    codec: " << audio_format.codec();
        qDebug() << "    byte order: " << audio_format.byteOrder();
        qDebug() << "    sample size: " << audio_format.sampleSize();
        qDebug() << "    sample type: " << audio_format.sampleType();
        qDebug() << "    channels: " << audio_format.channels();
        audio_out = NULL;
        delete audio_output;
    }
}

void Audio::select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder) {
    qDebug() << "selected audio " << info.deviceName() <<  " sampleRate:" << rate << " Channels: " << channels << " Endian:" << (byteOrder==QAudioFormat::BigEndian?"BigEndian":"LittleEndian");

    sampleRate=rate;
    audio_channels=channels;
    audio_byte_order=byteOrder;

    if(audio_output!=NULL) {
        audio_output->stop();
        audio_output->disconnect(this);
        delete audio_output;
    }

    audio_device=info;
    audio_format.setFrequency(sampleRate+(sampleRate==8000?SAMPLE_RATE_FUDGE:0));
    audio_format.setChannels(audio_channels);
    audio_format.setByteOrder(audio_byte_order);

    if (!audio_device.isFormatSupported(audio_format)) {
        qDebug()<<"Audio format not supported by device.";
    }

    audio_output = new QAudioOutput(audio_device, audio_format, this);
    connect(audio_output,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));

    qDebug() << "QAudioOutput: error=" << audio_output->error() << " state=" << audio_output->state();

 //   audio_output->setBufferSize(1024*48);
    audio_out = audio_output->start();

    if(audio_output->error()!=0) {
        qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << audio_output->state();

        qDebug() << "Format:";
        qDebug() << "    sample rate: " << audio_format.frequency();
        qDebug() << "    codec: " << audio_format.codec();
        qDebug() << "    byte order: " << audio_format.byteOrder();
        qDebug() << "    sample size: " << audio_format.sampleSize();
        qDebug() << "    sample type: " << audio_format.sampleType();
        qDebug() << "    channels: " << audio_format.channels();
        audio_out = NULL;
    }
}

void Audio::stateChanged(QAudio::State State){
    switch (State) {
        case QAudio::StoppedState:
            if (audio_output->error() != QAudio::NoError) {
                qDebug() << "QAudioOutput: after start error=" << audio_output->error() << " state=" << State;
            break;
            }
        case QAudio::IdleState:
        case QAudio::SuspendedState:
        case QAudio::ActiveState:
        default:
 //           qDebug() << "QAudioOutput: state changed" << " state=" << State;
        return;
    }
}

void Audio::set_audio_encoding(int enc){
    audio_encoding = enc;
}

void Audio::process_audio(char* header,char* buffer,int length) {
    //qDebug() << "process audio";
    int written=0;
    int length_to_write, total_to_write;

    if (audio_encoding == 0) aLawDecode(buffer,length);
    else if (audio_encoding == 1) pcmDecode(buffer,length);
    else if (audio_encoding == 2) codec2Decode(buffer,length);
    else {
        qDebug() << "Error:  audio_encoding = " << audio_encoding;
    }

    if(audio_out!=NULL) {
        //qDebug() << "writing audio data length=: " <<  decoded_buffer.length();
        total_to_write = decoded_buffer.length();
        while( written< total_to_write) {
            if (audio_output->bytesFree() < 4) usleep(1000);
            length_to_write = (audio_output->periodSize() > (decoded_buffer.length()-written)) ?
                        (decoded_buffer.length()-written) : audio_output->periodSize();
            written+=audio_out->write(&decoded_buffer.data()[written],length_to_write);
        }
    }

    if (header != NULL) free(header);
    if (buffer != NULL) free(buffer);
}

void Audio::aLawDecode(char* buffer,int length) {
    int i;
    short v;

    //qDebug() << "aLawDecode " << decoded_buffer.length();
    decoded_buffer.clear();

    for (i=0; i < length; i++) {
        v=decodetable[buffer[i]&0xFF];

        switch(audio_byte_order) {
        case QAudioFormat::LittleEndian:
            decoded_buffer.append((char)(v&0xFF));
            decoded_buffer.append((char)((v>>8)&0xFF));
            break;
        case QAudioFormat::BigEndian:
            decoded_buffer.append((char)((v>>8)&0xFF));
            decoded_buffer.append((char)(v&0xFF));
            break;
        }
    }

}

void Audio::pcmDecode(char* buffer,int length) {
    int i;

    decoded_buffer.clear();

    for (i=0; i < length; i+=2) {           // try reducing the audio output

        switch(audio_byte_order) {
        case QAudioFormat::LittleEndian:
            decoded_buffer.append(buffer[i]);
            decoded_buffer.append(buffer[i+1]);
            break;
        case QAudioFormat::BigEndian:
            decoded_buffer.append(buffer[i+1]);
            decoded_buffer.append(buffer[i]);
            break;
        }
    }

}

void Audio::codec2Decode(char* buffer,int length) {
    int i,j;
    short v[CODEC2_SAMPLES_PER_FRAME];
    unsigned char bits[BITS_SIZE];

    //qDebug() << "codec2wDecode " << decoded_buffer.length();

    decoded_buffer.clear();

    j = 0;
    while (j < length) {
        memcpy(bits,&buffer[j],BITS_SIZE);
        codec2_decode(codec2, v, bits);

        for (i=0; i < CODEC2_SAMPLES_PER_FRAME; i++)
        switch(audio_byte_order) {
        case QAudioFormat::LittleEndian:
            decoded_buffer.append((char)(v[i]&0xFF));
            decoded_buffer.append((char)((v[i]>>8)&0xFF));
            break;
        case QAudioFormat::BigEndian:
            decoded_buffer.append((char)((v[i]>>8)&0xFF));
            decoded_buffer.append((char)(v[i]&0xFF));
            break;
        }
        j += BITS_SIZE;
    }

}

void Audio::init_decodetable() {
    qDebug() << "init_decodetable";
    for (int i = 0; i < 256; i++) {
        int input = i ^ 85;
        int mantissa = (input & 15) << 4;
        int segment = (input & 112) >> 4;
        int value = mantissa + 8;
        if (segment >= 1) {
            value += 256;
        }
        if (segment > 1) {
            value <<= (segment - 1);
        }
        if ((input & 128) == 0) {
            value = -value;
        }
        decodetable[i] = (short) value;

    }
}


