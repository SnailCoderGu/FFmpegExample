extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}
#include "FileDecode.h"

int main() {
    // 注册所有的编解码器、格式和协议
    av_register_all();

    FileDecode fileDecode;
    int ret = fileDecode.OpenFile("oceans.mp4");
    if (ret != 0)
    {
        std::cout << "OpenFile Faild";
    }
    ret = fileDecode.OpenAudioDecode();
    ret = fileDecode.Decode();
    if (ret == 0)
    {
        std::cout << "解码音频完成" << std::endl;
    }
    return 0;
}
