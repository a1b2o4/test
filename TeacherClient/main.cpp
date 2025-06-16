#include "teacherclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TeacherClient w;
    w.show();
    return a.exec();
}
