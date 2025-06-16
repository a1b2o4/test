#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>

class Server : public QTcpServer {
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);
    ~Server();


    void handleRequest(QTcpSocket* client, const QJsonObject& message);
    void sendResponse(QTcpSocket* clientSocket, const QString& requestType, const QJsonObject& response = {});


    void startServer(quint16 port);


protected:
    void incomingConnection(qintptr socketDescriptor) override;


private slots:
    void onReadyRead();
    void onClientDisconnected();


private:
    QJsonArray defaultSettings;

    QMap<QTcpSocket*, QString> clients; // Храним клиентов и их роли (студент/преподаватель)
    QHash<QString, QJsonObject*> studentsT;   // hash - json если запрос от преподавателя
    QHash<QTcpSocket*, QJsonObject*> studentsS;    // сокет - json данные студента если запрос от студента

    QHash<QString, QJsonArray> studentsSettings;

    QJsonArray studentStructure;    // структура хранения данных студента


    void loadStudentStructure();
    void loadDefaultSettings();  // Метод для загрузки настроек


    void hGetStructure(QTcpSocket* client);
    void hAddStudent(QTcpSocket* client, const QJsonObject& data);
    void hGetStudentList(QTcpSocket* client);
    void hGetDefaultSettings(QTcpSocket* client);
//    void hRequestSettings(QTcpSocket* client);
    void hUpdateDefaultSettings(QTcpSocket* client, const QJsonObject& settings);
    void hUpdateStudentSettings(QTcpSocket* client, const QJsonObject& data); //для преподавателя
    void hStudentSelected(QTcpSocket* client, const QJsonObject& data);

    void hGetStudentSettings(QTcpSocket* client, const QJsonObject& data); //для преподавателя
    void hSendStudentData(QTcpSocket* client,  const QJsonObject& data);
    void hUpdateStudentsData(QTcpSocket* client);
    void hGetStudentData(QTcpSocket* client);
    void hUpdateStudentFields(QTcpSocket* client, const QJsonObject& data);
    void hShutdownServer(QTcpSocket* client);

    void saveDefaultSettings();
};

#endif // SERVER_H
