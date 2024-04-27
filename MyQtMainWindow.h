#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "YuvRender.h"

class MyQtMainWindow  : public QMainWindow
{
	Q_OBJECT

public:
	MyQtMainWindow(QWidget *parent);
	~MyQtMainWindow();

	void initData(int w, int h);
	void updateYuv(uint8_t* y, uint8_t* u, uint8_t* v);

	void Close();

private:
	Ui::MainWindow* ui = nullptr;

	YuvRender* render = nullptr;

};
