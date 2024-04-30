#include "ImageYuvRender.h"
#include "libyuv.h"
#include <QPainter>
#include <QDebug>

void ImageYuvRender::paintEvent(QPaintEvent* event) {

    if (yuvData == NULL || rgbaData == NULL)
    {
        return;
    }

    QPainter painter(this);

    QImage rgbImage(rgbaData,width_, height_, QImage::Format_RGBA8888);

    int window_with = size().width();
    int window_height = size().height();

    float window_rate = (float)window_with / (float)window_height;
    float imag_rate  = (float)width_ / (float)height_;

    int target_width;
    int target_height;
    int x;
    int y;
    
    if (window_rate > imag_rate)
    {
        target_height = window_height;
        target_width = target_height * imag_rate;
        x = (window_with - target_width)/2;
        y = 0;
    }
    else
    {
        target_width = window_with;
        target_height = target_width * height_ / width_;
        y = (window_height - target_height) / 2;
        x = 0;
    }
    QSize tszie(target_width,target_height);
    //qDebug() << "size() " << x << " , " << y <<" w " << target_width << " h " << target_height;

    //qDebug() << "size() " << x << " , " << y << " w " << size().width() << " h " << size().height();
    // ÔÚ´°¿ÚÖÐ»æÖÆRGBÍ¼Ïñ
    painter.drawImage(x, y, rgbImage.scaled(tszie, Qt::KeepAspectRatioByExpanding));

}



void ImageYuvRender::initData(int w, int h,int stride_width)
{
    width_ = w;
    height_ = h;
    stride_width_ = stride_width;

    allocData();

    qDebug() << "initData ok";
}

void ImageYuvRender::updateYuv(uint8_t* y, uint8_t* u, uint8_t* v)
{
    memcpy(yuvData, y, stride_width_ * height_);
    memcpy(yuvData + stride_width_ * height_, v, stride_width_ * height_ / 4);
    memcpy(yuvData + stride_width_ * height_ * 5 / 4, u, stride_width_ * height_ / 4);

    int size = stride_width_ * height_;
    libyuv::I420ToARGB(yuvData, stride_width_, yuvData + size, stride_width_ / 2,
        yuvData + size * 5 / 4, stride_width_ / 2,
        rgbaData, width_ * 4,
        width_, height_);

    this->update();
}

void ImageYuvRender::Close()
{
    if (yuvData)
    {
        delete[] yuvData;
        yuvData = nullptr;
    }
    if (rgbaData)
    {
        delete[] rgbaData;
        rgbaData = nullptr;
    }
    this->update();
}

void ImageYuvRender::allocData()
{

    const int rgbaSize = width_ * height_ * 4;
    if (yuvData == NULL)
    {
        const int size = stride_width_ * height_ * 3 / 2; // YUV420p  //yuv¿¼ÂÇ¿ç¾à
        yuvData = new uint8_t[size];
        memset(yuvData, 5, size);

        const int rgbaSize = width_ * height_ * 4;
        rgbaData = new uint8_t[rgbaSize];
        memset(rgbaData, 5, rgbaSize);
    }

    qDebug() << "initData ok";
}
