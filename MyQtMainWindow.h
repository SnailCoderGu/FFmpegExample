#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "ImageYuvRender.h"
#include <QSlider>
#include <QResizeEvent>
#include <QLabel>
#include <QFileDialog>
#include <thread>
#include <QTimer>
#include <FileDecode.h>

class MyQtMainWindow  : public QMainWindow
{
	Q_OBJECT
public:
	MyQtMainWindow(QWidget *parent);
	~MyQtMainWindow();


	void initData(int w, int h,int stride_w);
	void updateYuv(uint8_t* y, uint8_t* u, uint8_t* v);

	void ClosePlayer();


	void test();

	void closeEvent(QCloseEvent* event) override;

	

public slots:
	void OnSliderValueChanged(int value);
	void OnSliderPressed();
	void OnSliderValueReleased();

	void OpenFileDialog();
	void OpenNetUrlDialog();
	void UpdatePlayerInfo();
	void ClickPlay();
private:

	void ReleaseUI();
	
	Ui::MainWindow* ui = nullptr;


	QSlider* progressBar;
	QLabel* info_label;

	QTimer timer;
	QPushButton* stopButton;
	QPushButton* pushButton;

	FileDecode* fileDecode;
	
	ImageYuvRender* render = nullptr;

	bool playFlag = true;


};
