#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class YuvRender  : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	YuvRender(QWidget *parent);
	~YuvRender();

	void initializeGL() override;

	void resizeGL(int w, int h) override;
	void paintGL() override;

	void initData(int w, int h);

	void updateYuv(uint8_t* y, uint8_t* u, uint8_t* v);

	void Close();

private:
	void allocData();

	uint8_t* yuvData = NULL;
	uint8_t* rgbaData = NULL;

	int width_ = 0;      // Í¼Ïñ¿í¶È
	int height_ = 0;     // Í¼Ïñ¸ß¶È

	GLuint textureId;
};
