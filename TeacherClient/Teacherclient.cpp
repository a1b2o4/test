#include "teacherclient.h"
#include "studentinfo.h"
#include "variantselectiondialog.h"
#include "settingsdialog.h"
#include "groupsdialog.h"
#include "variantswidget.h"
#include "testswidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QLabel>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTextEdit>
#include <QTextStream>
#include <QMenuBar>
#include <QJsonArray>
#include <QVector>


TeacherClient::TeacherClient(QWidget* parent)
    : QMainWindow(parent), socket(new QTcpSocket(this)), settingsDialog(nullptr)
{
    connect(socket, &QTcpSocket::connected, this, &TeacherClient::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &TeacherClient::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &TeacherClient::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &TeacherClient::onErrorOccurred);

    connectToServer("127.0.0.1", 12345); // IP и порт сервера

    setupUI();
    createMenu();

    qDebug()<<"vload ";
    variants = readVariantsFromFile("variants.txt");

    //stSettings = loadSettingsFromTXT("standartSettings.txt");
    //qDebug() << "JSON-настройки:" << stSettings;//QJsonDocument(stSettings).toJson(QJsonDocument::Indented);
}

TeacherClient::~TeacherClient() {}

//+++++++++++++API

void TeacherClient::sendRequest(const QString& requestType, const QJsonObject& data, bool wait)
{
    QJsonObject request;
    request["role"] = "teacher";

    request["action"] = requestType;
    if (!data.isEmpty()) {
        request["data"] = data;
    }

    QJsonDocument doc(request);
    QByteArray requestData = doc.toJson();

    socket->write(requestData);
    socket->flush();

    qDebug() << "Запрос отправлен на сервер.";
    qDebug() << doc;

    if ((wait) && (socket->waitForReadyRead(5000)))
    {
        qDebug() << "Данные получены, waitForReadyRead разблокировался";
        qDebug() << "Получен ответ.";
    } else {
        qDebug() << "Ошибка: Тайм-аут при ожидании данных";
    }
}

void TeacherClient::handleResponse(const QByteArray& responseData)
{
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    qDebug() << "doc формат ответа!" <<doc;

    if (!doc.isObject())
    {
        qDebug() << "Некорректный формат ответа!";
        return;
    }

    QJsonObject response = doc.object();

    // Определяем тип ответа
    QString action = response["action"].toString();
    qDebug() << "Получен ответ от сервера. Action:" << action;

    if (response.contains("data"))    // Дальше можно обрабатывать данные, если они есть
    {
        qDebug()<<"contains(data) ";
        if (action == "getStudentStructure")
        {
            QJsonObject data = response["data"].toObject();
            qDebug()<<"data1 " << data;
            studentInfo::loadStudentFields(data);
        }
        else if (action == "updateStudentsData")
        {
            QJsonObject data = response["data"].toObject();
            UpdateStudentsData(data["studentsData"].toArray());
        }
        else if (action == "getDefaultSettings")
        {
            QJsonObject data = response["data"].toObject();
            recDefaultSettings(data["defaultSettings"].toArray());
            qDebug() << "].toArray():" << data["defaultSettings"].toArray();
            qDebug() << "data:" << data;
        }
        else if (action == "getStudentList")
        {
            QJsonArray studentArray = response["StudentList"].toArray();
            UpdateStudentsData(studentArray);  // Обновляем данные студентов
        }
        else
        {
            qDebug() << "Неизвестный action в ответе сервера:" << action;
        }
    }
}

//-------------API



//+++++++++++++СЛОТЫ И СИГНАЛЫ СЕРВЕРА

void TeacherClient::onConnected()
{
    qDebug("Подключено к серверу!");
    getStudentStructure();
    getDefaultSettings();

   // sendRequest("updateStudentsData");
}

void TeacherClient::onReadyRead()
{
    qDebug() << "принят ответ:";
    QByteArray data = socket->readAll();
    handleResponse(data);
}

void TeacherClient::onDisconnected()
{
    qDebug("Соединение с сервером разорвано.");
}

void TeacherClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    QString errorMessage;
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError:
        errorMessage = "Сервер не найден.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        errorMessage = "Соединение отклонено сервером.";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMessage = "Сервер разорвал соединение.";
        break;
    default:
        errorMessage = "Ошибка: " + socket->errorString();
    }

    QMessageBox::critical(this, "Ошибка подключения", errorMessage);
    qDebug("Ошибка подключения.");
}

//-------------СЛОТЫ И СИГНАЛЫ СЕРВЕРА



//+++++++++++++ИНТЕРФЕЙС

    //+++++++++++++СОЗДАНИЕ ИНТЕРФЕЙСА

void TeacherClient::setupUI()
{
    // Главный виджет с вкладками
    auto* mainTabs = new QTabWidget(this);
    setCentralWidget(mainTabs);

    // --------- Вкладка "Администрирование"
    adminWidget = new QWidget(this);
    auto* adminTabs = new QTabWidget(adminWidget);

    groupsTab = new GroupsWidget(this);
    variantsTab = new VariantsWidget(this);
    testsTab = new TestsWidget(this);

    adminTabs->addTab(groupsTab, "Группы студентов");
    adminTabs->addTab(variantsTab, "Варианты");
    adminTabs->addTab(testsTab, "Тесты");

    auto* adminLayout = new QVBoxLayout(adminWidget);
    adminLayout->addWidget(adminTabs);

    // --------- Вкладка "Настройка"
    settingsWidget = new QWidget(this);
    // пока пусто — можно добавить позже

    // --------- Вкладка "Наблюдение"
    monitorWidget = new QWidget(this);
    auto* monitorTabs = new QTabWidget(monitorWidget);

    studentSelectionTab = new StudentSelectionWidget(this);
    studentObservationTab = new QWidget(this);   // содержимое старого UI

    auto* obsLayout = new QHBoxLayout(studentObservationTab);

    // Левая панель — таблица студентов
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    setupStudentTable(leftLayout);
    leftPanel->setLayout(leftLayout);
    obsLayout->addWidget(leftPanel, 2);

    // Правая панель — информация о студенте
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);
    studentInfoTextEdit = new QTextEdit(this);
    studentInfoTextEdit->setReadOnly(true);
    rightLayout->addWidget(studentInfoTextEdit);
    rightPanel->setLayout(rightLayout);
    obsLayout->addWidget(rightPanel, 1);

    // Кнопка "Обновить список студентов"
    QPushButton* refreshButton = new QPushButton("Обновить список студентов", this);
    leftLayout->addWidget(refreshButton);
    connect(refreshButton, &QPushButton::clicked, this, &TeacherClient::updateStudentsFromServer);

    // Добавляем во вкладки наблюдения
    monitorTabs->addTab(studentSelectionTab, "Выбор студентов");
    monitorTabs->addTab(studentObservationTab, "Наблюдение за работой");

    auto* monitorLayout = new QVBoxLayout(monitorWidget);
    monitorLayout->addWidget(monitorTabs);

    // --------- Добавляем все 3 основные вкладки
    mainTabs->addTab(adminWidget, "Администрирование");
    mainTabs->addTab(monitorWidget, "Наблюдение");
    mainTabs->addTab(settingsWidget, "Настройка");
}



/*
void TeacherClient::setupUI()
{
    auto* mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    auto* layout = new QHBoxLayout(mainWidget);    // Основной layout

    // Панель слева (Список студентов)
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);

    setupStudentTable(leftLayout);    // Создаем таблицу с данными студентов

    leftPanel->setLayout(leftLayout);
    layout->addWidget(leftPanel, 2);  // Панель слева занимает 2 части

    // Панель справа (Вкладки для выбранного студента)
    auto* rightPanel = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightPanel);

    studentInfoTextEdit = new QTextEdit(this);
    studentInfoTextEdit->setReadOnly(true); // Делаем поле только для чтения

    rightLayout->addWidget(studentInfoTextEdit);
    rightPanel->setLayout(rightLayout);
    layout->addWidget(rightPanel, 1);  // Панель справа занимает 1 часть

    // Кнопки
    //QPushButton* sendStudentsButton = new QPushButton("Отправить список студентов", this);
    //layout->addWidget(sendStudentsButton);

    QPushButton* requestStudentsConnectButton = new QPushButton("Обновить список студентов", this);
    layout->addWidget(requestStudentsConnectButton);
    connect(requestStudentsConnectButton, &QPushButton::clicked, this, &TeacherClient::updateStudentsFromServer);
}*/

void TeacherClient::setupStudentTable(QVBoxLayout* layout)
{
    studentTable = new QTableWidget(this);

    QStringList headers;
    QList<int> hiddenColumns;

    int column = 0;
    for (const QJsonValue& value : studentInfo::studentFields)
    {
        if (value.isObject())  // Проверяем, что это QJsonObject
        {
            QJsonObject field = value.toObject();  // Извлекаем объект

            headers.append(field["label"].toString()); // Добавляем заголовки для таблицы
            qDebug() << "добавленные столбцы " << field["label"];

            if (!field["showInTable"].toBool()) // Если поле не нужно показывать в таблице, запоминаем его
            {
                hiddenColumns.append(column);
                qDebug() << "скрытые столбцы " << field["label"];
            }
            column++;
        }
    }

    studentTable->setColumnCount(headers.size());
    studentTable->setHorizontalHeaderLabels(headers);

    for (int col : hiddenColumns) // Скрываем столбцы, которые не нужно показывать
    {
        studentTable->setColumnHidden(col, true);
    }

    studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    //studentTable->setSelectionMode(QAbstractItemView::SingleSelection);
    studentTable->setColumnHidden(0, true); // Скрываем столбец с хэшем (предполагается, что это первый столбец)
    connect(studentTable, &QTableWidget::itemSelectionChanged, this, &TeacherClient::onStudentSelected);
    connect(studentTable, &QTableWidget::cellDoubleClicked, this, &TeacherClient::onVariantDoubleClicked);
    layout->addWidget(studentTable);
}

void TeacherClient::createMenu()
{
    auto* menuBar = new QMenuBar(this);

    // Меню "Файл"
    auto* fileMenu = menuBar->addMenu("Файл");

    // Пункт "Добавить студентов"
    //QAction* addStudentsAction = new QAction("Добавить студентов", this);
    //connect(addStudentsAction, &QAction::triggered, this, &TeacherClient::onAddStudents);
    //fileMenu->addAction(addStudentsAction);

    // Пункт "Изменить настройки"
    QAction* openAction = new QAction("Изменить настройки", this);
    connect(openAction, &QAction::triggered, this, &TeacherClient::onOpenSettings);
    fileMenu->addAction(openAction);

    // Пункт "Изменить настройки"
    QAction* shutdownServerAction = new QAction("Завершить работу сервера", this);
    connect(shutdownServerAction, &QAction::triggered, this, &TeacherClient::shutdownServer);
    fileMenu->addAction(shutdownServerAction);

    // Меню "Студент"
    auto* studentMenu = menuBar->addMenu("Студент");
    QAction* editStudentSettings = new QAction("Изменить настройки студента", this);
    QAction* editStudentVariant = new QAction("Изменить вариант студента", this);

    connect(editStudentSettings, &QAction::triggered, this, &TeacherClient::onEditStudentSettings);
    connect(editStudentVariant, &QAction::triggered, this, &TeacherClient::onEditStudentVariant);
    studentMenu->addAction(editStudentSettings);
    studentMenu->addAction(editStudentVariant);

    QAction* groupsAction = new QAction("Группы студентов", this);
    connect(groupsAction, &QAction::triggered, this, &TeacherClient::onOpenGroups);
    fileMenu->addAction(groupsAction);

    setMenuBar(menuBar);
}

void TeacherClient::onOpenGroups() {
    GroupsDialog dlg(this);
    dlg.setGroupsFile("groups.json");
    dlg.setStudentsFile("students.json");
    connect(&dlg, &GroupsDialog::groupStudentNames, this, &TeacherClient::onAddStudents);
    dlg.loadGroups();
    dlg.loadStudents();
    if (dlg.exec() == QDialog::Accepted) {
        // всё обработается в сигналах
    }
}

    //-------------СОЗДАНИЕ ИНТЕРФЕЙСА


    //+++++++++++++ОБРАБОТКА СОБЫТИЙ

void TeacherClient::onEditStudentVariant()
{
    int row = studentTable->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Ошибка", "Выберите студента в таблице.");
        return;
    }

    // Найти колонку с вариантом, используя label из studentFields
    QString variantLabel;
    for (const QJsonValue& fieldValue : studentInfo::studentFields)
    {
        QJsonObject field = fieldValue.toObject();
        if (field["id"].toString() == "variant")
        {
            variantLabel = field["label"].toString();
            break;
        }
    }

    int variantColumn = -1;
    for (int i = 0; i < studentTable->columnCount(); ++i)
    {
        QTableWidgetItem* headerItem = studentTable->horizontalHeaderItem(i);
        if (headerItem && headerItem->text() == variantLabel)
        {
            variantColumn = i;
            break;
        }
    }

    if (variantColumn == -1)
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось найти колонку варианта.");
        return;
    }

    openVariantSelectionDialog(row, variantColumn);
}


void TeacherClient::shutdownServer()
{
    QJsonObject request;
    sendRequest("shutdownServer", request, true); // если у тебя есть sendJson для отправки на сервер
}

void TeacherClient::onEditStudentSettings()
{
    QModelIndexList selectedIndexes = studentTable->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите студента для изменения настроек.");
        return;
    }

    // Используем `QSet`, чтобы исключить дублирующиеся строки
    QSet<int> selectedRows;
    for (const QModelIndex& index : selectedIndexes) {
        selectedRows.insert(index.row());
    }

    // Берём первую выделенную строку
    int firstRow = *selectedRows.begin();
    QTableWidgetItem* firstItem = studentTable->item(firstRow, 0);
    if (!firstItem) {
        QMessageBox::warning(this, "Ошибка", "Ошибка при получении данных студента.");
        return;
    }

    QString primaryStudentHash = firstItem->data(Qt::UserRole).toString();
    qDebug() << "primaryStudentHash: " << primaryStudentHash;

    // Загружаем настройки студента или берём defaultSettings
    QJsonArray initialSettings = studentSettings.contains(primaryStudentHash)
                                     ? studentSettings[primaryStudentHash]
                                     : defaultSettings;
    qDebug() << "Загружены настройки для студента: " << initialSettings;

    // Если окно настроек ещё не создано — создаём, иначе обновляем его содержимое
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(initialSettings, this);
    } else {
        settingsDialog->updateSettings(initialSettings);
    }

    // Открываем окно настроек
    if (settingsDialog->exec() == QDialog::Accepted) {
        QJsonArray updatedSettings = settingsDialog->getUpdatedSettings();
        QJsonArray studentHashes;

        // Добавляем хэши **только уникальных строк**
        for (int row : selectedRows) {
            QTableWidgetItem* studentItem = studentTable->item(row, 0);
            if (!studentItem) continue;

            QString currentStudentHash = studentItem->data(Qt::UserRole).toString();
            if (!currentStudentHash.isEmpty()) {
                studentHashes.append(currentStudentHash);
                studentSettings[currentStudentHash] = updatedSettings;
            }
        }

        // Отправляем обновления на сервер
        sendUpdatedStudentsSettings(studentHashes, updatedSettings);
        qDebug() << "Обновлённые настройки отправлены на сервер.";
    }
}


void TeacherClient::sendUpdatedStudentsSettings(const QJsonArray& studentHashes, const QJsonArray& updatedSettings)
{
    QJsonObject request;
    request["studentHashes"] = studentHashes;
    request["updatedSettings"] = updatedSettings;

    sendRequest("updateStudentSettings", request);
    qDebug() << "Отправлены обновленные настройки на сервер:" << request;
}

void TeacherClient::sendUpdatedSettings(const QJsonArray& updatedSettings)
{
    QJsonObject request;
    //defaultSettings = settingsDialog->getUpdatedSettings();
    request["defaultSettings"] = updatedSettings;

    sendRequest("updateDefaultSettings", request);
    qDebug() << "Отправлены обновлённые настройки на сервер";
}

void TeacherClient::onOpenSettings()
{
    qDebug() << "Открытие настроек по умолчанию, текущие defaultSettings: " << defaultSettings;

    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(defaultSettings, this);
    } else {
        settingsDialog->updateSettings(defaultSettings);
    }

    if (settingsDialog->exec() == QDialog::Accepted)
    {
        defaultSettings = settingsDialog->getUpdatedSettings();
        sendUpdatedSettings(defaultSettings);  // Отправляем обновлённые настройки на сервер
    }

    qDebug() << "Настройки по умолчанию обновлены: " << defaultSettings;
}

void TeacherClient::onSaveSettings() {
    // Реализуйте сохранение настроек
}

void TeacherClient::onAddStudents(const QStringList& names, QString group)
{

   /* QVector<studentInfo*> newStudents = studentInfo::readStudentListFromCSV();

    for (studentInfo* student : newStudents)
    {
        QString studentHash = student->getHash();
        qDebug() << studentHash;

        if (students.contains(studentHash))
        {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Подтверждение",
                QString("Студент %1 уже существует. Продолжить добавление?")
                    .arg(student->getStudentsData()["name"].toString()),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No) {
                delete student;  // Освобождаем память
                continue;
            }
        }

        addStudent(student);        // Добавляем студента в список
        updateStudentRow(student->getStudentsData());  // Обновляем его строку в таблице
    }

    qDebug() << "Добавлено студентов: " << newStudents.size();

    sendNewStudentListToServer(newStudents);*/



   qDebug() << names;


    QVector<studentInfo*> newStudents;

    for (const QString& name : names)
    {
        QJsonObject studentData;

        studentData.insert("name", name);  // Заполняем QJsonObject
        studentData.insert("group", group);  // Заполняем QJsonObject

        studentInfo* newStudent = new studentInfo(studentData);  // Передаем QJsonObject в конструктор
        newStudents.append(newStudent);
    }

    for (studentInfo* student : newStudents)
    {
        QString studentHash = student->getHash();
        qDebug() << studentHash;

        if (students.contains(studentHash))
        {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Подтверждение",
                QString("Студент %1 уже существует. Продолжить добавление?")
                    .arg(student->getStudentsData()["name"].toString()),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No) {
                delete student;  // Освобождаем память
                continue;
            }
        }

        addStudent(student);        // Добавляем студента в список
        updateStudentRow(student->getStudentsData());  // Обновляем его строку в таблице
    }

    qDebug() << "Добавлено студентов: " << newStudents.size();

    sendNewStudentListToServer(newStudents);

}

void TeacherClient::onStudentSelected()
{
    int row = studentTable->currentRow();
    if (row == -1) return;  // Если ничего не выбрано, выходим

    QString studentHash = studentTable->item(row, 0)->data(Qt::UserRole).toString();
    QString studentInfo = students.value(studentHash)->getStudentInfo();

    studentInfoTextEdit->setText(studentInfo);    // Заполняем текстовое поле
}

void TeacherClient::onVariantDoubleClicked(int row, int column)
{
    QTableWidgetItem* headerItem = studentTable->horizontalHeaderItem(column);
    if (!headerItem) return;

    QString columnHeader = headerItem->text();

    // Получаем label столбца "variant" из структуры хранения данных студента
    QString variantLabel;
    for (const QJsonValue& fieldValue : studentInfo::studentFields)
    {
        QJsonObject field = fieldValue.toObject();
        if (field["id"].toString() == "variant")
        {
            variantLabel = field["label"].toString();
            break;
        }
    }

    if (columnHeader == variantLabel)
    {
        qDebug() << "Variant column clicked. Opening selection window...";
        openVariantSelectionDialog(row, column); // Передаем и строку, и колонку
    }
}

    //-------------ОБРАБОТКА СОБЫТИЙ



    //+++++++++++++ДИАЛОГОВЫЕ ОКНА

void TeacherClient::openVariantSelectionDialog(int row, int column)
{
    QTableWidgetItem* hashItem = studentTable->item(row, 0);
    qDebug() << "!hashItem "<<!hashItem<< hashItem->data(Qt::UserRole).toString();
    if (!hashItem) return;

    QTableWidgetItem* item = studentTable->item(row, column);
    QString currentVariant = item ? item->text() : "";


    qDebug() << "Opening variant selection for row" << row << ", column" << column << "Current variant:" << currentVariant;

    QString studentHash = hashItem->data(Qt::UserRole).toString();
    studentInfo* student = students[studentHash];


    qDebug() << "Attempting to open variant dialog...";
    VariantSelectionDialog dialog(getVariants(), this, currentVariant);
    qDebug() << "Before exec";
    if (dialog.exec() == QDialog::Accepted)
    {
        qDebug() << "Dialog accepted";
        QString newVariant = dialog.getSelectedVariant();

        student->setVariant(newVariant);  // Обновляем в studentInfo
        updateStudentRow(student->getStudentsData());  // Обновляем строку в таблице

        //sendStudentData(student->getStudentsData());
        updateSelectedStudentFields(studentHash, QStringList("variant"));

    }
    qDebug() << "After exec";
}


    //-------------ДИАЛОГОВЫЕ ОКНА

//-------------ИНТЕРФЕЙС



//+++++++++++++ОТПРАВКА ЗАПРОСОВ

void TeacherClient::updateStudentsFromServer()
{

    sendRequest("updateStudentsData"); // Отправляем запрос на сервер
}

void TeacherClient::sendStudentData(const QJsonObject& studentData)
{
    if (studentData.isEmpty())
    {
        qDebug() << "Ошибка: Пустые данные студента!";
        return;
    }

    qDebug() << "Отправка данных студента" << studentData << "на сервер...";
    sendRequest("sendStudentData", studentData);
}

void TeacherClient::sendNewStudentListToServer(QVector<studentInfo*> newStudents)
{
    QJsonArray studentArray;

    for (studentInfo* student : newStudents)
    {
        if (!student) continue;

        QJsonObject studentObject;

        foreach (QString key, student->getStudentsData().keys())
        {
            QString value = student->getStudentsData().value(key).toString();
            studentObject[key] = value;
            qDebug() << key <<value;
        }

        studentArray.append(studentObject);
    }

    QJsonObject requestData;
    requestData["students"] = studentArray;

    sendRequest("addStudent", requestData);  // Отправка на сервер
}

void TeacherClient::getDefaultSettings()
{
    sendRequest("getDefaultSettings");
    qDebug() << "Отправлен запрос на получение настроек по умолчанию";
}

void TeacherClient::getStudentStructure()
{
    sendRequest("getStudentStructure");
    qDebug() << "Отправлен запрос на получение структуры ханения данных студента";
}

void TeacherClient::updateSelectedStudentFields(const QString& studentHash, const QStringList& selectedFieldIds)
{
    if (!students.contains(studentHash))
    {
        qWarning() << "Студент не найден.";
        return;
    }

    studentInfo* student = students[studentHash];
    QJsonObject fullData = student->getStudentsData();
    QJsonObject updatePayload;

    for (const QString& fieldId : selectedFieldIds)
    {
        if (fullData.contains(fieldId)) {
            updatePayload[fieldId] = fullData[fieldId];
        }
    }

    updatePayload["hash"] = studentHash;  // обязательно

    sendRequest("updateStudentFields", updatePayload);
}





//-------------ОТПРАВКА ЗАПРОСОВ



//+++++++++++++ОБРАБОТКА ОТВЕТОВ СЕРВЕРА

void TeacherClient::UpdateStudentsData(const QJsonArray& studentArray)
{
    qDebug() <<  "UpdateStudentsData ";// << studentArray;
    for (const QJsonValue& value : studentArray)
    {
        if (!value.isObject()) continue;

        QJsonObject studentObj = value.toObject();
        QString studentHash = studentObj["hash"].toString();

        studentInfo* student;

        if (students.contains(studentHash))
        {
            student = students[studentHash];
        }
        else
        {
            student = new studentInfo(studentObj);
            students.insert(studentHash, student);
        }

        QJsonObject updatedJsonObject;
        for (const QString& key : studentObj.keys())
        {
            updatedJsonObject[key] = studentObj[key].toString();
        }

        student->setStudentsData(updatedJsonObject);
        student->setConnected(studentObj["connected"].toBool());
        qDebug() <<  "updateRow " << student->getStudentsData();
        updateStudentRow(student->getStudentsData()); // Обновляем строку в таблице
    }
}

void TeacherClient::recDefaultSettings(const QJsonArray& response)
{
    qDebug() << "Получены настройки по умолчанию:" << response;

    // Сохраняем настройки в переменную
    defaultSettings = response;
    qDebug() << "response:" << response;
    qDebug() << "defaultSettings:" << defaultSettings;
}

//-------------ОБРАБОТКА ОТВЕТОВ СЕРВЕРА



//+++++++++++++ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ

void TeacherClient::connectToServer(const QString& host, quint16 port)
{
    socket->connectToHost("127.0.0.1", 12345); // Подключение к локальному серверу
    if (socket->waitForConnected(3000))
    {
        QMessageBox::information(this, "Подключение", "Соединение установлено!");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу!");
    }
}

void TeacherClient::addStudent(studentInfo* newStudent)
{
    students.insert(newStudent->getHash(), newStudent);
}

void TeacherClient::updateStudentTable()
{
    studentTable->clearContents();
    studentTable->setRowCount(0);

    for (auto student : students)
    {
        updateStudentRow(student->getStudentsData());
    }
}

void TeacherClient::updateStudentRow(const QJsonObject& studentsData)
{
    if (studentsData.isEmpty()) return;

    QString studentHash = studentsData["hash"].toString();
    if (studentHash.isEmpty()) {
        qDebug() << "Ошибка: В данных студента отсутствует hash!";
        return;
    }

    int row = -1;

    // 🔍 Ищем строку с таким же хэшем в 0-м столбце
    for (int i = 0; i < studentTable->rowCount(); i++)
    {
        QTableWidgetItem* item = studentTable->item(i, 0);
        if (item && item->data(Qt::UserRole).toString() == studentHash)
        {
            row = i;
            break;
        }
    }

    // Если не нашли строку, создаем новую
    if (row == -1)
    {
        row = studentTable->rowCount();
        studentTable->insertRow(row);
    }

    QString cellValue;
    QString fieldId;
    int column = -1;

    // 🔵 Обновляем или добавляем данные в строку
    for (const QJsonValue& fieldValue : studentInfo::studentFields)
    {
        column++;
        if (!fieldValue.isObject()) continue;  // Пропускаем, если это не объект

        QJsonObject field = fieldValue.toObject();

        fieldId = field["id"].toString();
        cellValue = studentsData.contains(fieldId) ? studentsData.value(fieldId).toString() : field["defaultValue"].toString();

        if (fieldId == "connected")
        {
            // Преобразуем boolean в иконки для статуса
            cellValue = studentsData["connected"].toString() == "true" ? "🟢" : "🔴";
        }

        if (fieldId == "variant")
        {
            QStringList parts = studentsData["variant"].toString().split("-", Qt::SkipEmptyParts);
            cellValue = parts[0];
        }

        // Если поле в строке уже существует, то обновляем его значение
        QTableWidgetItem* existingItem = studentTable->item(row, column);
        if (existingItem)
        {
            existingItem->setText(cellValue);
        }
        else
        {
            // Если это новый элемент, то создаем новый
            QTableWidgetItem* newItem = new QTableWidgetItem(cellValue);
            newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);  // Сделаем элемент не редактируемым
            studentTable->setItem(row, column, newItem);
        }

        // Увековечиваем хэш в качестве пользовательских данных
        if (fieldId == "hash") {
            studentTable->item(row, 0)->setData(Qt::UserRole, studentHash);
        }
    }

    qDebug() << "Данные студента обновлены: " << studentHash;
}

QStringList TeacherClient::readVariantsFromFile(const QString& filePath)
{
    QStringList variantsList;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Не удалось открыть файл:" << filePath;
        return variantsList; // Возвращаем пустой список
    }

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();

        if (!line.isEmpty())
        {
            variantsList.append(line); // Добавляем строку в список
        }
    }

    file.close();
    return variantsList;
}

//-------------ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ








//-------------------------------------------------
/*void TeacherClient::loadStudentSettings(QString studentHash) {
    // Загружаем настройки для выбранного студента
    /*settingsTable->clearContents();

    // Получаем список настроек (примерные данные)
    QJsonObject studentSettings = getStudentSettings(studentID);
    settingsTable->setRowCount(studentSettings.size() + 1);

    // Первый элемент - статус применены ли настройки
    settingsTable->setItem(0, 0, new QTableWidgetItem("Применены ли настройки"));
    bool settingsApplied = studentSettings["applied"].toBool();
    settingsTable->setItem(0, 1, new QTableWidgetItem(settingsApplied ? "✅ Да" : "❌ Нет"));

    // Добавляем остальные настройки
    int row = 1;
    for (auto it = studentSettings.begin(); it != studentSettings.end(); ++it) {
        settingsTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        settingsTable->setItem(row, 1, new QTableWidgetItem(it.value().toString()));
        row++;
    }/
}
*/
void TeacherClient::loadStudentVariants(QString studentHash) {
    // Загружаем варианты студента
    /*variantsTable->clearContents();

    // Получаем варианты (примерные данные)
    QVector<QPair<int, QString>> variants = getStudentVariants(studentID);
    variantsTable->setRowCount(variants.size());

    for (int i = 0; i < variants.size(); ++i) {
        int variantNumber = variants[i].first;
        QString variantText = variants[i].second;

        variantsTable->setItem(i, 0, new QTableWidgetItem(QString::number(variantNumber)));
        variantsTable->setItem(i, 1, new QTableWidgetItem(variantText));
    }*/
}

QJsonObject TeacherClient::loadSettingsFromTXT(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Ошибка открытия TXT файла!";
        return {};
    }

    QJsonObject settings;
    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed(); // Убираем пробелы

        // Пропускаем комментарии и пустые строки
        if (line.isEmpty() || line.startsWith("//"))
            continue;

        // Разделяем строку по '='
        QStringList parts = line.split(' ');
        if (parts.size() != 2)
        {
            qDebug() << "Ошибка в строке (пропускаем):" << line;
            continue;
        }

        QString key = parts[0].trimmed();
        QString value = parts[1].trimmed();

        // Определяем тип значения
        if (value.toLower() == "true" || value.toLower() == "false")
        {
            settings[key] = (value.toLower() == "true");
        }
        else
        {
            bool isNumber;
            int intValue = value.toInt(&isNumber);
            if (isNumber)
                settings[key] = intValue;
            else
                settings[key] = value; // Оставляем строкой
        }
    }

    file.close();
    return settings;
}

bool TeacherClient::saveSettingsToTXT(const QJsonObject& settings)
{
    // Открываем диалог выбора папки
    QString folderPath = QFileDialog::getExistingDirectory(nullptr, "Выберите папку для сохранения");

    // Проверяем, выбрал ли пользователь папку
    if (folderPath.isEmpty())
    {
        qDebug() << "Сохранение отменено пользователем.";
        return false;
    }

    QString filePath = folderPath + "/settings.txt"; // Имя файла

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Ошибка при сохранении файла!";
        return false;
    }

    QTextStream out(&file);

    // Записываем данные
    for (const QString& key : settings.keys())
    {
        out << key << " = " << settings.value(key).toVariant().toString() << "\n";
    }

    file.close();
    qDebug() << "Настройки сохранены в:" << filePath;

    return true;
}




