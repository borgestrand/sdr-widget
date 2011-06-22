/* 
 * File:   audio.h
 * Author: john
 *
 * Created on 06 August 2010, 16:17
 */

#ifndef AUDIO_H
#define	AUDIO_H

#include <QtCore>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtGui/QComboBox>

class audio : public QObject {
    Q_OBJECT
public:
    audio();
    audio(const audio& orig);
    virtual ~audio();
    void initialize_audio(int buffer_size);
    void select_audio(QAudioDeviceInfo info);
    void process_audio(char* buffer);
    void get_audio_devices(QComboBox* comboBox);

private:
    void aLawDecode(char* buffer);
    void init_decodetable();
    QAudioFormat     audio_format;
    QAudioOutput*    audio_output;
    QAudioDeviceInfo audio_device;
    QIODevice*       audio_out;
    QByteArray       decoded_buffer;
    short decodetable[256];

};

#endif	/* AUDIO_H */

