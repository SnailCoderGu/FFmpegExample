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
        format.setSampleRate(rate); // 采样率
        format.setChannelCount(nch);   // 声道数
        format.setSampleSize(sample_size);    // 采样大小
        format.setCodec("audio/pcm"); // 音频编码格式
        format.setByteOrder(QAudioFormat::LittleEndian); // 字节顺序
        format.setSampleType(QAudioFormat::SignedInt);  // 采样类型

        int len = dst_nb_samples * format.channelCount() * format.sampleSize()/8;

       
         // 创建 QAudioOutput 对象
        audioOutput = new QAudioOutput(format);

        audioOutput->setVolume(1.0); // 设置音量（0.0 - 1.0）

        // 打开音频输出
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




