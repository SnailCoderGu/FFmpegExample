#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class ChooseUrlUi; }
QT_END_NAMESPACE

class ChooseUrlDialog  : public QDialog
{
	Q_OBJECT

public:
	ChooseUrlDialog(QWidget *parent = nullptr);
	~ChooseUrlDialog();
private slots:
	void onOkButtonClicked(); 
		

private:
	Ui::ChooseUrlUi* ui;
public:
	std::string url;
};
