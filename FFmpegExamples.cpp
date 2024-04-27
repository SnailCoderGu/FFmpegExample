extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}
#include "FileDecode.h"
#include <qapplication.h>
#include "MyQtMainWindow.h"
#include <thread>

void runFFmepg(MyQtMainWindow* mywindow);

int main(int argc, char* argv[]) {


    QApplication app(argc, argv);

    MyQtMainWindow* qwindow = new MyQtMainWindow(NULL);
    qwindow->resize(960, 400);
    qwindow->show();
    std::thread t1(runFFmepg, qwindow);

    app.exec();


}

void runFFmepg(MyQtMainWindow* mywindow){
    // 注册所有的编解码器、格式和协议
    av_register_all();

    FileDecode fileDecode;
    int ret = fileDecode.AVOpenFile("oceans.mp4");
    if (ret != 0)
    {
        std::cout << "AVOpenFile Faild";
    }
    fileDecode.SetMyWindow(mywindow);
    ret = fileDecode.OpenVideoDecode();
    ret = fileDecode.OpenAudioDecode();
    

    ret = fileDecode.Decode();
    if (ret == 0)
    {
        std::cout << "解码音频完成" << std::endl;
    }
    fileDecode.Close();
    mywindow = NULL;
}
