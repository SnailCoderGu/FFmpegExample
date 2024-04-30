#pragma once
#include <QWidget>
#include <QImage>
class ImageYuvRender :public QWidget
{
    Q_OBJECT
public:
    ImageYuvRender(QWidget* parent = nullptr) : QWidget(parent) {

    }

    void initData(int w, int h, int stride_width);

    void updateYuv(uint8_t* y, uint8_t* u, uint8_t* v);

    void Close();
protected:
    void paintEvent(QPaintEvent* event) override;

private:

    void allocData();

    uint8_t* yuvData = NULL;
    uint8_t* rgbaData = NULL;

    int width_ = 0;      // 图像宽度
    int stride_width_ = 0; //lineszie 可能大于width
    int height_ = 0;     // 图像高度

};

