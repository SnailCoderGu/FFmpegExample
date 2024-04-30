

#include <QApplication>
#include "MyQtMainWindow.h"


int main(int argc, char* argv[]) {


    QApplication app(argc, argv);

    MyQtMainWindow* qwindow = new MyQtMainWindow(NULL);
    qwindow->show();

    app.exec();
}

