#ifdef _WIN32
#include <windows.h>
#endif
#include <QCoreApplication>

#include "server.h"

void customMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg) {
    QTextStream out(stdout);

    out.setEncoding(QStringConverter::Utf8);

    switch (type)
    {
    case QtDebugMsg:
        //out << "[DEBUG] " << msg << Qt::endl;
        break;
    case QtInfoMsg:
        out << "[INFO] " << msg << Qt::endl;
        break;
    case QtWarningMsg:
        out << "[WARNING] " << msg << Qt::endl;
        break;
    case QtCriticalMsg:
        out << "[CRITICAL] " << msg << Qt::endl;
        break;
    case QtFatalMsg:
        out << "[FATAL] " << msg << Qt::endl;
        abort();
    }
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    //SetConsoleOutputCP(CP_UTF8);
#endif
    //qInstallMessageHandler(customMessageHandler); // <-- вот это важно

    QCoreApplication a(argc, argv);

    Server server;
    server.startServer(12345); // Сервер запускается на порту 12345

    return a.exec();
}
