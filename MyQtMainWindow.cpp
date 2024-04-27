#include "MyQtMainWindow.h"
#include <QVBoxLayout>


MyQtMainWindow::MyQtMainWindow(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
	
{
	ui->setupUi(this);

	QVBoxLayout* layout = new QVBoxLayout(ui->centralwidget);
	render = new YuvRender(ui->centralwidget);

	layout->addWidget(render);
}

MyQtMainWindow::~MyQtMainWindow()
{}

void MyQtMainWindow::initData(int w, int h)
{
	render->initData(w, h);
}

void MyQtMainWindow::updateYuv(uint8_t* y, uint8_t* u, uint8_t* v)
{
	render->updateYuv(y, u, v);
}

void MyQtMainWindow::Close()
{
	render->Close();
}
