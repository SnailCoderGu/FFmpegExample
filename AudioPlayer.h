#pragma once

#include <QApplication>
#include <QAudioOutput>
#include <QIODevice>
#include <QDebug>

class AudioPlayer {
public:
    AudioPlayer() {

    }

    void SetFormat(int dst_nb_samples, int rate, int sample_size, int nch) {
        QAudioFormat format;
        format.setSampleRate(rate); // ������
        format.setChannelCount(nch);   // ������
        format.setSampleSize(sample_size);    // ������С
        format.setCodec("audio/pcm"); // ��Ƶ�����ʽ
        format.setByteOrder(QAudioFormat::LittleEndian); // �ֽ�˳��
        format.setSampleType(QAudioFormat::SignedInt);  // ��������

        int len = dst_nb_samples * format.channelCount() * format.sampleSize()/8;

       
         // ���� QAudioOutput ����
        audioOutput = new QAudioOutput(format);

        audioOutput->setVolume(1.0); // ����������0.0 - 1.0��

        // ����Ƶ���
        outputDevice = audioOutput->start();
    }

    void writeData(const char* data, qint64 len) {
        //audioData.insert(0, data, len);

        outputDevice->write(data, len);
    }

    void Close()
    {

        if (outputDevice != nullptr)
        {
            outputDevice->close();
            delete outputDevice;
            outputDevice = NULL;
        }
        if (audioOutput)
        {
            //audioOutput->stop();
            audioOutput = NULL;
        }

        

    }

private:
    QIODevice* outputDevice;
   
    QAudioOutput* audioOutput;
};



