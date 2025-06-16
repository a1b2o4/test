#include "mainwindow.h"
#include "studentinputdialog.h"
#include "studentinfo.h"
#include "variantinfo.h"
#include "testdialog.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow *w = new MainWindow();  // Создаем главное окно


    qDebug() << "testdialogex"<< w->getTestDialog();


    w->getTestDialog()->startTest();

    return a.exec();

}
