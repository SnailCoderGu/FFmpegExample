#include "YuvRender.h"
#include <QDebug>
#include "libyuv.h"

YuvRender::YuvRender(QWidget *parent)
	: QOpenGLWidget(parent)
{}

YuvRender::~YuvRender()
{}

void YuvRender::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // 创建纹理对象
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	qDebug() << "initializeGL ok";
}

void YuvRender::resizeGL(int w, int h)
{
    glViewport(0, 0, width_, height_); // 设置视口大小
    qDebug() << "resizeGL ok" << width_ << "-" << height_;
}

void YuvRender::paintGL()
{
    if (yuvData == NULL || rgbaData == NULL)
    {
        return;
    }
    // 更新纹理数据
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);
    // 绘制纹理
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2f(-1.0, -1.0);
    glTexCoord2f(1.0, 0.0); glVertex2f(1.0, -1.0);
    glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
    glTexCoord2f(0.0, 1.0); glVertex2f(-1.0, 1.0);
    glEnd();
}

void YuvRender::initData(int w, int h)
{
	width_ = w;
	height_ = h;

    allocData();

    qDebug() << "initData ok";
}

void YuvRender::updateYuv(uint8_t* y, uint8_t* u, uint8_t* v)
{
    memcpy(yuvData, y, width_ * height_);
    memcpy(yuvData + width_ * height_, v, width_ * height_ / 4);
    memcpy(yuvData + width_ * height_ * 5 / 4, u, width_ * height_ / 4);

    int size = width_ * height_;
    libyuv::I420ToARGB(yuvData, width_, yuvData + size, width_ / 2,
        yuvData + size * 5 / 4, width_ / 2,
        rgbaData, width_ * 4,
        width_, height_);

    this->update();
}

void YuvRender::Close()
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
}

void YuvRender::allocData()
{

    const int rgbaSize = width_ * height_ * 4;
    if (yuvData == NULL)
    {
        const int size = width_ * height_ * 3 / 2; // YUV420p
        yuvData = new uint8_t[size];
        memset(yuvData, 5, size);

        const int rgbaSize = width_ * height_ * 4;
        rgbaData = new uint8_t[rgbaSize];
        memset(rgbaData, 5, rgbaSize);
    }

    qDebug() << "initData ok";
}
