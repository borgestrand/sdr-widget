/* 
 * File:   Audio.h
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

#ifndef AUDIO_H
#define	AUDIO_H

#include <QtCore>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtGui/QComboBox>

#define AUDIO_BUFFER_SIZE 480

#define BIGENDIAN

// There are problems running at 8000 samples per second on Mac OS X
// The resolution is to run at 8011 samples persecond.
#define SAMPLE_RATE_FUDGE 11

class Audio : public QObject {
    Q_OBJECT
public:
    Audio();
    Audio(const Audio& orig);
    virtual ~Audio();
    void initialize_audio(int buffer_size);
    void select_audio(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    void process_audio(char* header,char* buffer,int length);
    void get_audio_devices(QComboBox* comboBox);

    int get_sample_rate();
    int get_channels();

private:
    void aLawDecode(char* buffer,int length);
    void init_decodetable();
    QAudioFormat     audio_format;
    QAudioOutput*    audio_output;
    QAudioDeviceInfo audio_device;
    QIODevice*       audio_out;
    QByteArray       decoded_buffer;
    short decodetable[256];

    int sampleRate;
    int audio_channels;
    QAudioFormat::Endian audio_byte_order;

};

#endif	/* AUDIO_H */
