#include "server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QJsonArray>

#include <QCoreApplication>

Server::Server(QObject* parent)
    : QTcpServer(parent)
{
    loadStudentStructure();
    loadDefaultSettings();
}

Server::~Server() {}

//+++++++++++++СЛОТЫ И СИГНАЛЫ СЕРВЕРА
void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* clientSocket = new QTcpSocket(this);
    if (!clientSocket->setSocketDescriptor(socketDescriptor))
    {
        qDebug() << "Ошибка установки дескриптора сокета!";
        delete clientSocket;
        return;
    }

    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);

    clients[clientSocket] = "unknown"; // По умолчанию клиент без роли
    qDebug() << "Новое подключение от:" << clientSocket->peerAddress().toString();
    qDebug() << "Роль" << clients[clientSocket];
}

void Server::onReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QByteArray data = client->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    /*if (!jsonDoc.isObject())
    {
        qDebug() << "Получены некорректные данные от клиента.";
        qDebug() << jsonDoc;
        return;
    }*/

    QJsonObject message = jsonDoc.object();

    // Определяем роль клиента
    if (clients[client] == "unknown" && message.contains("role"))
    {
        if (message["role"] == "teacher")
        {
            clients[client] = "teacher";
            qDebug() << "Клиент определён как преподаватель.";
        } else if (message["role"] == "student")
        {
            clients[client] = "student";
            qDebug() << "Клиент определён как студент.";
        }
    }

    handleRequest(client, message);
}

void Server::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    if (studentsS.contains(client))
    {
        QJsonObject* studentData = studentsS[client];
        if (studentData)
        {
            studentData->insert("connected", false);  // Обновляем значение
        }
        studentsS.remove(client); // Удаляем из хэша
    }

    qDebug() << "Клиент отключился:" << clients[client];
    clients.remove(client);
    client->deleteLater();

}

//-------------СЛОТЫ И СИГНАЛЫ СЕРВЕРА


//+++++++++++++API

void Server::sendResponse(QTcpSocket* client, const QString& requestType, const QJsonObject& responseData)
{
    QJsonObject response;
    response["action"] = requestType;  // Тип запроса
    response["data"] = responseData;   // Данные запроса

    QJsonDocument doc(response);
    QByteArray responseDataBytes = doc.toJson();

    qDebug() << "Отправка ответа клиенту:" << client->peerAddress().toString();
    qDebug() << "Ответ JSON:" << doc;

    client->write(responseDataBytes);
    client->flush();
}

void Server::handleRequest(QTcpSocket* client, const QJsonObject& message)
{
    qDebug() << "message " << message;
    if (!message.contains("action"))
    {
        qDebug() << "Ошибка: отсутствует requestType в запросе!";
        return;
    }

    QString requestType = message["action"].toString();
    qDebug() << "action. " << requestType;
    qDebug() << "data. " << message["data"].toObject();

    if (requestType == "getStudentStructure")
    {
        hGetStructure(client);
    }
    else if (requestType == "addStudent")
    {
        hAddStudent(client, message["data"].toObject());
    }
    else if (requestType == "getStudentList")
    {
        hGetStudentList(client);
    }
    else if (requestType == "studentSelected")
    {
        hStudentSelected(client, message["data"].toObject());
    }
    else if (requestType == "updateStudentsData")
    {
        hUpdateStudentsData(client);
    }
    else if (requestType == "sendStudentData")
    {
        hSendStudentData(client, message["data"].toObject());
    }
    else if (requestType == "getDefaultSettings")
    {
        hGetDefaultSettings(client);
    }
    else if (requestType == "updateDefaultSettings")
    {
        hUpdateDefaultSettings(client, message["data"].toObject());
    }
    else if (requestType == "updateStudentSettings") {
        hUpdateStudentSettings(client, message["data"].toObject());
    }
    else if (requestType == "getStudentSettings") {
        hGetStudentSettings(client, message["data"].toObject());
    }
    else if (requestType == "getStudentData") {
        hGetStudentData(client);
    }
    else if (requestType == "updateStudentFields") {
        hUpdateStudentFields(client, message["data"].toObject());
    }
    else if (requestType == "shutdownServer") {
        hShutdownServer(client);
    }
    else
    {
        qDebug() << "Неизвестный тип запроса:" << requestType;
    }
}

//-------------API


//+++++++++++++ОБРАБОТЧИКИ ЗАПРОСОВ

void Server::hSendStudentData(QTcpSocket* client, const QJsonObject& data)
{
    QString studentHash = data["hash"].toString();

    if (data.isEmpty())
    {
        qDebug() << "Ошибка: Пустые данные студента!";
        return;
    }

    if (studentHash.isEmpty())
    {
        qDebug() << "Ошибка: Пустые данные студента!";
        //sendResponse(client, "sendStudentData", "Error: Hash is missing!");
        return;
    }

    if (studentsT.contains(studentHash))
    {
        qDebug() << "studentsT[studentHash]:" << *(studentsT[studentHash]);
        *(studentsT[studentHash]) = data;  // Перезаписываем существующий объект
        qDebug() << "data:" << *(studentsT[studentHash]);
        qDebug() << "Student data updated for hash:" << studentHash;
    } else {
        qDebug() << "Error: Student not found!";
    }

    sendResponse(client, "sendStudentData");    // Отправляем подтверждение клиенту
}

void Server::hUpdateStudentSettings(QTcpSocket* client, const QJsonObject& data)
{
    QJsonArray settings = data["updatedSettings"].toArray();
    QJsonArray studentHashes = data["studentHashes"].toArray();

    for (const QJsonValue& value : studentHashes)
    {
        QString studentHash = value.toString();
        studentsSettings[studentHash] = settings;  // Теперь ключ - это строка, а не QTcpSocket*
    }

    qDebug() << "Обновлены настройки для студентов:" << studentHashes;
    qDebug() << "studentsSettings:" << studentsSettings;
    sendResponse(client, "getStudentStructure");
}

void Server::hGetStudentSettings(QTcpSocket* clientSocket, const QJsonObject& request)
{
    QString studentHash = request["studentHash"].toString();

    QJsonObject response;
    if (studentsSettings.contains(studentHash))
    {
        response["settings"] = studentsSettings[studentHash];  // Отправляем настройки студента
    } else
    {
        response["settings"] = defaultSettings;  // Если настроек нет, отправляем пустой массив
    }

    sendResponse(clientSocket, "getStudentSettings", response);
}

void Server::hGetStructure(QTcpSocket* client)
{
    if (studentStructure.isEmpty())
    {
        qDebug() << "Ошибка: структура данных не загружена!";
        return;
    }

    QJsonObject responseData;
    responseData["structure"] = QJsonArray(studentStructure);
    //qDebug()<<"studentStructure " << studentStructure;

    sendResponse(client, "getStudentStructure", responseData);
}

void Server::hStudentSelected(QTcpSocket* client, const QJsonObject& data)
{
    QJsonObject* studentInfo = studentsT.value(data["hash"].toString(), nullptr);

    if (studentInfo) // Проверяем, что студент найден
    {
        studentInfo->insert("connected", true); // Изменяем значение на true
        studentsS[client] = studentInfo; // Связываем сокет с объектом студента

        qDebug()<<"studentInfo->insert(connected, true)" <<studentsS[client]->value("connected"); // Связываем сокет с объектом студента


        QJsonObject response;
        response["studentFields"] = studentStructure;
        response["studentData"] = *studentInfo;

        qDebug() << "Отправка данных студента клиенту:" << response;

        sendResponse(client, "studentSelected", response); // Отправляем подтверждение
    }
    else
    {
        sendResponse(client, "Error: Student not found");
    }
}

void Server::hAddStudent(QTcpSocket* client, const QJsonObject& data)
{
    QJsonArray studentArray = data["students"].toArray();
    qDebug() << "Добавление студентов:" << studentArray;

    QJsonArray updatedStudents;

    for (const QJsonValue& value : studentArray)
    {
        if (value.isObject())
        {
            QJsonObject studentObj = value.toObject();
            QString studentHash = studentObj["hash"].toString();

            // Проверка: студент с таким хэшем уже есть
            if (studentsT.contains(studentHash)) {
                qDebug() << "Студент с hash" << studentHash << "уже существует. Отправляем его данные.";

                // Если студент уже существует, добавляем его данные в список
                QJsonObject* existingStudentObj = studentsT[studentHash];
                updatedStudents.append(*existingStudentObj);

                continue;  // Переходим к следующему студенту
            }

            // Если студент новый, добавляем его в список
            QJsonObject* newStudentObj = new QJsonObject(studentObj);
            studentsT[studentHash] = newStudentObj;

            qDebug() << "Добавлен студент:" << *newStudentObj;
            updatedStudents.append(*newStudentObj);  // Добавляем нового студента в список обновленных
        }
    }

    qDebug() << "Список студентов обновлён.";

    // Отправляем ответ с данными студентов (новые или существующие)
    QJsonObject response;
    response["studentsData"] = updatedStudents;  // Включаем студентов в ответ
    sendResponse(client, "updateStudentsData", response);}

void Server::hUpdateStudentsData(QTcpSocket* client) // информация о подключенных студентах возвращается
{
    QJsonArray connectedStudentsArray;
    for (auto it = studentsT.begin(); it != studentsT.end(); ++it)
    {
        QJsonObject* studentData = it.value();
        if (studentData)
        {
            connectedStudentsArray.append(*studentData);
            qDebug() << "studentsS"<<*studentData;
        }
    }

    QJsonObject response;
    response["studentsData"] = connectedStudentsArray;

    sendResponse(client, "updateStudentsData", response);
}

void Server::hGetStudentList(QTcpSocket* client)
{
    QJsonArray studentArray;

    foreach (QString studentHash, studentsT.keys())
    {
        QJsonObject filteredStudent;  // Объект с отфильтрованными данными

        QJsonObject* studentObj = studentsT[studentHash];
        if (studentObj)
        {
            for (const QJsonValue& value : studentStructure) // Используем QJsonValue
            {
                QJsonObject field = value.toObject(); // Преобразуем QJsonValue в QJsonObject
                QString fieldId = field["id"].toString();
                bool sendToStudent = field["sendToStudent"].toBool();

                if (sendToStudent && studentObj->contains(fieldId))
                {
                    filteredStudent[fieldId] = (*studentObj)[fieldId];
                }
            }
            studentArray.append(filteredStudent);
        }
    }

    QJsonObject response;
    response["StudentList"] = studentArray;

    sendResponse(client, "getStudentList", response);
}

void Server::hGetDefaultSettings(QTcpSocket* client)
{
    QJsonObject response;
    if (defaultSettings.isEmpty())
    {
        response["Error"] = "Настройки по умолчанию отсутствуют";

        qDebug() << "Ошибка: Настройки по умолчанию отсутствуют";
        sendResponse(client, "getDefaultSettings", response);
        return;
    }

    response["defaultSettings"] = defaultSettings;

    sendResponse(client, "getDefaultSettings", response); // Отправляем настройки
    qDebug() << "Отправлены настройки по умолчанию преподавателю";
}

void Server::hUpdateDefaultSettings(QTcpSocket* client, const QJsonObject& data)
{
    QJsonArray newSettings = data["defaultSettings"].toArray();

    if (newSettings.isEmpty()) {
        QJsonObject response;
        response["Error"] = "Настройки по умолчанию отсутствуют";

        sendResponse(client, "Error", response);
        return;
    }

    defaultSettings = newSettings;
    saveDefaultSettings();  // Сохраняем в файл

    sendResponse(client, "defaultSettingsUpdated");
    //qDebug() << "Настройки по умолчанию обновлены";
}

void Server::hGetStudentData(QTcpSocket* client)
{
    if (!studentsS.contains(client))
    {
        sendResponse(client, "Error", QJsonObject{{"message", "Student not associated with this socket"}});
        return;
    }

    QJsonObject* studentInfo = studentsS[client];
    if (!studentInfo) {
        sendResponse(client, "Error", QJsonObject{{"message", "Student info not found"}});
        return;
    }

    QJsonObject response;
    response["studentFields"] = studentStructure;
    response["studentData"] = *studentInfo;

    sendResponse(client, "getStudentData", response);
}

void Server::hUpdateStudentFields(QTcpSocket* client, const QJsonObject& data)
{
    QString studentHash = data["hash"].toString();
    if (!studentsT.contains(studentHash))
    {
        qDebug() << "Ошибка: Студент не найден!";
        sendResponse(client, "Error", QJsonObject{{"message", "Student not found"}});
        return;
    }

    QJsonObject* student = studentsT[studentHash];
    // Обновляем все поля, кроме "hash"
    for (QString key : data.keys())
    {
        if (key == "hash") continue;
        student->insert(key, data[key]);
        qDebug() << "Обновлено поле" << key << "->" << data[key].toString();
    }

    sendResponse(client, "studentFieldUpdated", QJsonObject{{"updatedFields", data}});
}

void Server::hShutdownServer(QTcpSocket* client)
{
    qDebug() << "Получена команда shutdownServer от преподавателя. Завершение работы...";

    QJsonObject response;
    response["status"] = "ok";
    response["message"] = "Сервер завершает работу";
    sendResponse(client, "shutdownServer", response);

    QCoreApplication::quit();
}

//-------------ОБРАБОТЧИКИ ЗАПРОСОВ


//+++++++++++++ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ

void Server::startServer(quint16 port)
{
    if (this->listen(QHostAddress::Any, port))
    {
        qDebug() << "Сервер запущен на порту:" << port;
    } else
    {
        qDebug() << "Не удалось запустить сервер!";
    }
}

void Server::loadStudentStructure()
{
    QFile file("students_structure.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Ошибка открытия файла структуры данных!";
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(fileData);

    if (!doc.isArray()) {
        qDebug() << "Ошибка: структура данных не является массивом!";
        return;
    }

    studentStructure = doc.array();  // Просто загружаем весь массив сразу

    if (studentStructure.isEmpty())
    {
        qDebug() << "Ошибка: структура данных не загружена!";
        return;
    }

    qDebug() << "Структура данных загружена на сервер:" << studentStructure;
}

void Server::loadDefaultSettings()
{
    QFile file("defaultSettings.json"); // Открываем файл настроек
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Ошибка: Не удалось открыть файл defaultSettings.json";
        return;
    }

    QByteArray jsonData = file.readAll(); // Читаем содержимое файла
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isArray()) {
        qDebug() << "Ошибка: Некорректный JSON в defaultSettings.json";
        return;
    }

    defaultSettings = doc.array(); // Сохраняем настройки в переменной
    qDebug() << "Настройки по умолчанию загружены:" << defaultSettings;
}

void Server::saveDefaultSettings() {
    QFile file("defaultSettings.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Ошибка сохранения настроек в файл";
        return;
    }

    QJsonDocument doc(defaultSettings);
    file.write(doc.toJson());
    file.close();
}

//-------------ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
