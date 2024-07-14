#include "ChooseUrlDialog.h"
#include <QMessageBox>
#include <QRegularExpression>
#include "ui_chooseurl.h"

ChooseUrlDialog::ChooseUrlDialog(QWidget *parent)
	: QDialog(parent),
	ui(new Ui::ChooseUrlUi)
{
	ui->setupUi(this);

	ui->lineEdit->setText("rtmp://192.168.109.128:1935/live/test");
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	connect(ui->okButton, &QPushButton::clicked, this, &ChooseUrlDialog::onOkButtonClicked);


}

void ChooseUrlDialog::onOkButtonClicked() {
	QString text = ui->lineEdit->text();
	if (text.isEmpty())
	{
		QMessageBox::warning(this, "Invalid Input", "Please enter a valid url address.");
		reject();
	}
	else
	{
		url = text.toStdString();
		accept();
	}
	//QRegularExpression re("^rtmp://.*$");
	//if (!re.match(text).hasMatch()) {
	//	QMessageBox::warning(this, "Invalid Input", "Please enter a valid RTMP address.");
	//	reject();
	//}
	//else {
	//	url = text.toStdString();
	//	accept();
	//}
}

ChooseUrlDialog::~ChooseUrlDialog()
{}
