#include <QtWidgets>
#include <QItemDelegate>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDialog>
#include <QLabel>
#include <iostream>
#include <QAction>
#include <QMenuBar>
#include <QJsonArray>

#include "mainwindow.h"
#include "qswitchingelement.h"
#include "qioport.h"
#include "qconnection.h"
#include "studentinfo.h"
#include "elementselector.h"
#include "propertiestable.h"
#include "variantinfodialog.h"
#include "variantinfo.h"
#include "taskinfo.h"
#include "studentinputdialog.h"
#include "startschemeinputdialog.h"
#include "testdialog.h"
#include "newgraphicsview.h"
#include "scheme.h"
#include "port.h"

PropertiesTable* MainWindow::_propertiesTable = nullptr;

MainWindow::MainWindow(/*StudentInfo* studentInfo, VariantInfo* variantInfo, */QWidget *parent)
    : QMainWindow(parent), socket(new QTcpSocket(this)), /*studentInfo(studentInfo), variantInfo(variantInfo), */scene(nullptr)
{
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onErrorOccurred);

    //connect(this, &MainWindow::studentsListLoaded, this, &MainWindow::showStudentSelectionDialog);
    //connect(this, &MainWindow::studentListCreate, this, &MainWindow::studentListCreated);

    connectToServer("127.0.0.1", 12345); // IP и порт сервера
/*
    QString filePath = "";
    qDebug() << "studentInputDialog:" ;

    StudentInputDialog* studentInputDialog = new StudentInputDialog(studentsList, nullptr, filePath);    // Создаем окно ввода данных

    if (studentInputDialog->exec() == QDialog::Accepted)    // Показываем окно модально
    {
        selectedStudentInfo = studentInputDialog->getStudent();  // Получаем объект Student из диалог
        studentSelected();
        qDebug() << "studentSelected:" ;

        reqStudentSettings();
        qDebug() << "reqStudentSettings:" ;
        //если нет подключения к серверу, то загрузить вариант из файла (переместив условие в конструктор)

        // вариант выдается после прохождения теста

        //QString variantFilePath = "v1.txt";
        //variantInfo = new VariantInfo(variantFilePath, true);
    }
    else
    {
        QApplication::quit(); // Закрываем приложение
    }

    testDialog = new TestDialog();
    
    connect(testDialog, &TestDialog::sendStudentData, studentInfo, &StudentInfo::setStudentData);
    connect(testDialog, &TestDialog::requestSetting, this, &MainWindow::provideSetting);
    connect(this, &MainWindow::sendSetting, testDialog, &TestDialog::receiveSetting);
    connect(testDialog, &TestDialog::testFinished, this, &MainWindow::onTestFinished);

*/

    /*QHBoxLayout *layout = new QHBoxLayout;
    view = new QGraphicsView();
    layout->addWidget(view);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);

    setCentralWidget(widget);*/


   // this->resize(1000, 800);  // Устанавливаем размер окна 800x600 пикселей

}


MainWindow::~MainWindow() {}

//+++++++++++++API

void MainWindow::sendRequest(const QString& requestType, bool wait, const QJsonObject& data)
{
    //сделать один метод для отправки и получнния данных студента, в данных передавать список полей и значения, тли только список полей
    QJsonObject request;
    request["role"] = "student";
    request["wait"] = wait;

    request["action"] = requestType;
    if (!data.isEmpty())
    {
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

        //QMessageBox::information(this, "Данные получены", "Данные получены!" + requestType);
        qDebug() << "Данные получены, waitForReadyRead разблокировался" + requestType;
        qDebug() << "Получен ответ.";
        QByteArray data = socket->readAll();

        handleResponse(data);
    } else {
        qDebug() << "Ошибка: Тайм-аут при ожидании данных";
    }
}

void MainWindow::handleResponse(const QByteArray& responseData)
{
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject())
    {
        qDebug() << "Некорректный формат ответа!";
        return;
    }

    QJsonObject response = doc.object();

    QString action = response["action"].toString();    // Определяем тип ответа
    qDebug() << "Получен ответ от сервера. Action:" << action;
    qDebug() << "Получен ответ от сервера. doc:" << doc;

    //if (response.contains("data"))    // Дальше можно обрабатывать данные, если они есть
    if (action == "getStudentList")
    {
        if (response["data"].isObject())
        {
            hGetStudentList(response["data"].toObject());
            //studentInputDialog = new StudentInputDialog(studentsList, nullptr/*, filePath*/);    // Создаем окно ввода данных

        }
    }
    else if ((action == "studentSelected") || (action == "getStudentData"))
    {
        if (response["data"].isObject())
        {
            hGetStudentData(response["data"].toObject());
        }
    }
    else if (action == "getStudentSettings")
    {
        if (response["data"].isObject())
        {
            hGetStudentSettings(response["data"].toObject());
        }
    }
    else
    {
        qDebug() << "Неизвестный action в ответе сервера:" << action;
    }
}

//-------------API



//+++++++++++++СЛОТЫ И СИГНАЛЫ СЕРВЕРА

void MainWindow::onConnected()
{
    qDebug("Подключено к серверу!");
    QMessageBox::information(this, "Подключение", "Соединение установлено!");
}

void MainWindow::onReadyRead()
{
    qDebug() << "принят ответ:";
    QByteArray data = socket->readAll();
    handleResponse(data);
}

void MainWindow::onDisconnected()
{
    qDebug("Соединение с сервером разорвано.");
}

void MainWindow::onErrorOccurred(QAbstractSocket::SocketError socketError)
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



//+++++++++++++ОТПРАВКА ЗАПРОСОВ

void MainWindow::studentSelected()
{
    QJsonObject currStudent;
    qDebug() << "selectedStudentInfo: "<<selectedStudentInfo;
    currStudent["hash"] = selectedStudentInfo["hash"];

    qDebug() << "studentSelected send: "<<currStudent;
    sendRequest("studentSelected", true, currStudent);
}

void MainWindow::reqStudentSettings()  // Запрос настроек с сервера
{
    QJsonObject request;
    qDebug() << "studentInfo->getStudentData() :" << studentInfo->getStudentData();

    request["studentHash"] = studentInfo->getStudentData()["hash"];  // Уникальный идентификатор студента
    qDebug() << "getStudentSettings send:" ;

    sendRequest("getStudentSettings", true, request);
}

void MainWindow::reqGetStudentData()
{
    sendRequest("getStudentData", true);
}

void MainWindow::updateSelectedStudentFields(const QStringList& selectedFieldIds)
{
    if (!studentInfo)
        return;

    // Получаем текущие данные студента в виде QMap<QString, QString>
    QMap<QString, QString> currentData = studentInfo->getStudentData();

    // Формируем объект для отправки только выбранных полей
    QJsonObject updateObj;
    for (const QString& key : selectedFieldIds) {
        if (currentData.contains(key)) {
            updateObj.insert(key, currentData[key]);
        }
    }
    // Добавляем идентификатор студента (hash) для идентификации на сервере
    if (currentData.contains("hash"))
        updateObj.insert("hash", currentData["hash"]);

    // Отправляем запрос на сервер; используем тип запроса "updateStudentField"
    sendRequest("updateStudentFields", true, updateObj);


    //qDebug() << "Отправлены обновлённые поля студента:" << updateObj;
}

//-------------ОТПРАВКА ЗАПРОСОВ



//+++++++++++++ОБРАБОТКА ОТВЕТОВ СЕРВЕРА

void MainWindow::hGetStudentList(const QJsonObject& responseData)
{
    QJsonArray studentArray = responseData["StudentList"].toArray();

    studentsList = studentArray;
    qDebug()<<"studentArray"<<studentsList;

    if (studentsList.empty())
    {
        {
            auto reply = QMessageBox::question(this, "Список пуст", "Список студентов пуст. Повторить попытку?",
                                               QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                sendRequest("getStudentList", true);
                qDebug() << "getStudentList return";
            }
            //QMessageBox::information(this, "getStudentListdata", "getStudentListdata!");
            else{
            qApp->quit();
            return;
            }
        }

    }
    else
    {
        studentListCreated();
    }


    //emit studentsListLoaded(); // Отправляем сигнал
}

void MainWindow::hGetStudentData(const QJsonObject& responseData)
{
    qDebug() << "Получены данные студента:" << responseData;

    StudentInfo::studentFields = responseData["studentFields"].toArray();
    QJsonObject studentDataJson = responseData["studentData"].toObject();

    studentInfo = new StudentInfo(studentDataJson);
    qDebug() << "Структура данных студента сохранена";

}

void MainWindow::hGetStudentSettings(const QJsonObject& settingsData)
{
    qDebug() << "hGetStudentSettings :" ;

    studentSettings.clear();  // Очищаем перед обновлением
    qDebug() << "hGetStudentSettings :settingsArray" ;

    QJsonArray settingsArray = settingsData["settings"].toArray();  // Получаем массив настроек

    for (const QJsonValue& value : settingsArray)
    {
        QJsonObject setting = value.toObject();  // Преобразуем элемент массива в объект

        QString key = setting["id"].toString();  // Название настройки
        QString val = setting["value"].toVariant().toString(); // Преобразование в строку
        QString type = setting["type"].toString(); // Тип настройки

        qDebug() << "key: " << key;
        qDebug() << "val: " << val << " " << setting["value"].toVariant(); // Теперь значение отображается
        qDebug() << "type: " << type;

        studentSettings[key] = qMakePair(val, type);
    }

    qDebug() << "Настройки студента загружены: " << studentSettings;
}

//-------------ОБРАБОТКА ОТВЕТОВ СЕРВЕРА



//+++++++++++++ИНТЕРФЕЙС

    //+++++++++++++СОЗДАНИЕ ИНТЕРФЕЙСА

void MainWindow::createActions()
{
    exitAction = new QAction(tr("Выход"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Quit Scenediagram example"));
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
}

void MainWindow::createDockWidgets()
{
    QHBoxLayout *layout = new QHBoxLayout;
    view = new NewGraphicsView();
    layout->addWidget(view);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    elementSelector = new ElementSelector(this);    // Создаём ElementSelector

    elementSelector->addButtonCell(QSwitchingElement::image(2, 2), tr("БКЭ"));    // Добавляем кнопки в ElementSelector
    elementSelector->addButtonCell(QConnection::image()          , tr("Соединение"));
    elementSelector->addButtonCell(QIOPort::Image(2, true)       , tr("Вход"));
    elementSelector->addButtonCell(QIOPort::Image(3, false)      , tr("Выход"));

    addDockWidget(Qt::LeftDockWidgetArea, elementSelector);    // Добавляем ElementSelector как dock widget

    // Создаем и добавляем PropertiesTable в главное окно
    PropertiesTable* _propertiesTable = new PropertiesTable();
    propertiesTable = _propertiesTable;
    addDockWidget(Qt::RightDockWidgetArea, _propertiesTable);

    MainWindow::_propertiesTable = propertiesTable;
}

void MainWindow::createMenu()
{
    fileMenu = menuBar()->addMenu(tr("&Файл"));
    itemMenu = menuBar()->addMenu(tr("&Правка"));
    aboutMenu = menuBar()->addMenu(tr("&Помощь"));
    testMenu = menuBar()->addMenu(tr("&Тест"));

    QMenu *modeMenu = menuBar()->addMenu("Режим работы схемы");
    QMenu *simulationMenu = menuBar()->addMenu(tr("&Запуск"));
    QMenu *taskMenu = menuBar()->addMenu(tr("&Задание"));

    QAction *openAction = fileMenu->addAction("Открыть");
    QAction *saveAction = fileMenu->addAction("Сохранить");
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QAction *insertAction = itemMenu->addAction("Вставить элемент");
    QAction *copyAction = itemMenu->addAction("Скопировать элемент");
    QAction *deleteAction = itemMenu->addAction("Удалить элемент");

    QAction *helpAction = aboutMenu->addAction("Справка");
    QAction *aboutAction = aboutMenu->addAction("О программе");


    validateAction = simulationMenu->addAction("Проверка схемы");
    simulateAction = simulationMenu->addAction("Моделирование блокировок");
    startSchemeAction = simulationMenu->addAction("Запустить схему");
    buildPathAction = simulationMenu->addAction("Построить маршрут");
    QAction *restartSchemeAction = simulationMenu->addAction("Сбросить запуск");

    QAction *startTestAction = testMenu->addAction("начать тест");
    QAction *showTestAction = testMenu->addAction("Результаты теста");

    QAction *taskInfoAction = taskMenu->addAction("Информация о варианте");

    QActionGroup *modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);

    editModeAction = new QAction("Редактирование", this);
    runModeAction = new QAction("Запуск схемы", this);                      // запрет редактирования схемы
    lockSimModeAction = new QAction("Моделирование блокировок", this);      // запрет редактирования схемы

    editModeAction->setCheckable(true);
    runModeAction->setCheckable(true);
    lockSimModeAction->setCheckable(true);

    modeGroup->addAction(editModeAction);
    modeGroup->addAction(runModeAction);
    modeGroup->addAction(lockSimModeAction);

    modeMenu->addAction(editModeAction);
    modeMenu->addAction(runModeAction);
    //modeMenu->addAction(lockSimModeAction);

    editModeAction->setChecked(true);

    connect(editModeAction, &QAction::triggered, this, &MainWindow::setEditMode);
    connect(runModeAction, &QAction::triggered, this, &MainWindow::setRunMode);
    connect(lockSimModeAction, &QAction::triggered, this, &MainWindow::setLockSimMode);


    // После переключения режима — обновить меню
    connect(modeGroup, &QActionGroup::triggered, this, [=]() {
        updateMenuState();
    });

    connect(restartSchemeAction, &QAction::triggered, this, [=]() {
        emit setSchemeMode(SchemeMode::Edit, false);
    });

    connect(startTestAction, &QAction::triggered, testDialog, &TestDialog::startTest);
    connect(showTestAction, &QAction::triggered, testDialog, &TestDialog::showTestResult);
    connect(taskInfoAction, &QAction::triggered, this, &MainWindow::showTaskInfoDialog);
    connect(buildPathAction, &QAction::triggered, this, &MainWindow::onBuildPath);

    connect(validateAction, &QAction::triggered, this, [=]() {
        QList<QString> errors = scene->checkScheme();
        qDebug() << "checkScheme " << errors;
        if (errors.empty())
        {
            setEditPass(true);
            QMessageBox::information(this, "", "Ошибок нет!");
        }
        else
        {
            setEditPass(false);
            int currentErrors = studentInfo->getStudentData()["errorsCount"].toInt();
            studentInfo->setStudentData("errorsCount", QString::number(currentErrors + 1));
            updateSelectedStudentFields(QStringList{"errorsCount"});
            QMessageBox::information(this, "", "Есть ошибки!");

        }
        updateMenuState();
    });

    connect(startSchemeAction, &QAction::triggered, this, &MainWindow::startScheme);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelectedItem);

    updateMenuState();
}

void MainWindow::onBuildPath()
{
    StartSchemeInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        buildPathPortNumber = dialog.getPortNumber();
        buildPathControlPacket = dialog.getControlPacket();

        if (buildPathPortNumber.isEmpty() || buildPathControlPacket.isEmpty())
        {
            QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены!");
            return;
        }

        bool ok;
        int portNum = buildPathPortNumber.toInt(&ok);
        if (!ok)
        {
            QMessageBox::warning(this, "Ошибка", "Некорректный номер входа!");
            return;
        }
        scene->setSchemeMode(SchemeMode::buildPath);
    }

}

void MainWindow::pathEnd()
{
    try {
        Port* startPort = scene->getScheme()->findPortByNumber(buildPathPortNumber);
        if (!startPort)
        {
            QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Ошибка", "Вход с таким номером не найден!");
            return;
        }

        if (!startPort) return;

        if (!scene->getScheme()->getTopology()->checkControlPacket(buildPathControlPacket))
        {
            QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Ошибка", "Ошибка управляющего пакета!");
            return;
        }
        bool isBlock;

            qDebug() << "buildPathControlPacket " << buildPathControlPacket;
        scene->getScheme()->getTokenPath(startPort, buildPathControlPacket, &isBlock);    // Получить пути соединений
        QList<Port*> rightPath = scene->getScheme()->getPathPorts();

        QList<Port*> buildPath = scene->getPathPorts();

        qDebug() << "buildPath.length() " << buildPath.length();
        qDebug() << "rightPath.length() " << rightPath.length();

        if (buildPath.length() == rightPath.length())
        {
            qDebug() << "= ";
            for (int i = 0; i < rightPath.length(); i++)
            {
                qDebug() << "i path " << i;
                qDebug() << "buildPath[i] " << buildPath[i];

                qDebug() << "buildPath[i] " << buildPath[i]->getFields();
                qDebug() << "rightPath[i] " << rightPath[i]->getFields();

                if (!(buildPath[i] == rightPath[i]))
                {
                    QMessageBox::warning(this, "Ошибка", QString("Путь составлен неверно"));
                    setLaunchPass(false);
                    return;
                }
            }

        }
        else
        {
            QMessageBox::warning(this, "Ошибка", QString("Путь составлен неверно"));
            setLaunchPass(false);
            return;
        }
        QMessageBox::information(this, "", QString("Путь составлен верно"));

        scene->setSchemeMode(SchemeMode::buildPath);
        scene->startScheme(buildPathPortNumber, buildPathControlPacket);
        //variantInfo->getCurrentTask()->setLaunchPass(true);
        setLaunchPass(true);
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(this, "Ошибка", QString("Произошла ошибка: %1").arg(e.what()));
        //variantInfo->getCurrentTask()->setLaunchPass(false);
        setLaunchPass(false);
    }
    catch (...)
    {
        QMessageBox::critical(this, "Ошибка", "Неизвестная ошибка");
        //variantInfo->getCurrentTask()->setLaunchPass(false);
        setLaunchPass(false);
    }
}

void MainWindow::updateModeAvailability()
{
    int taskIndex = variantInfo->getCurrentTaskNum() + 1;
    QString taskCheckedKey = QString("task%1Checked").arg(taskIndex);

    bool isChecked = studentInfo->getStudentData()[taskCheckedKey] == "true";
    qDebug() << "isChecked " << isChecked;

    //bool isChecked = variantInfo->getCurrentTask()->getEditPass();

    runModeAction->setEnabled(isChecked); // разрешён только если схема проверена
    lockSimModeAction->setEnabled(isChecked);  // моделирование можно вручную включать при необходимости

    editModeAction->setChecked((!isChecked && runModeAction->isChecked()) || editModeAction->isChecked());

}


void MainWindow::updateMenuState()
{

    if (editModeAction->isChecked())
    {
        validateAction->setEnabled(true);
        startSchemeAction->setEnabled(false);
        simulateAction->setEnabled(false);
    }
    else if (runModeAction->isChecked())
    {
        validateAction->setEnabled(false);
        startSchemeAction->setEnabled(true);
        simulateAction->setEnabled(true);
    }
    updateModeAvailability();
}

void MainWindow::setEditPass(bool Passed)
{
    qDebug() << "set edit pass";
    variantInfo->getCurrentTask()->setEditPass(Passed);
    QString taskId = QString("task%1Checked").arg(variantInfo->getCurrentTaskNum() + 1);
    studentInfo->setStudentData(taskId, Passed ? "true" : "false");
    updateSelectedStudentFields(QStringList{taskId});
    updateMenuState();
}

void MainWindow::setLaunchPass(bool Passed)
{
    qDebug() << "set launch pass " << Passed;
    variantInfo->getCurrentTask()->setLaunchPass(Passed);
    QString taskId = QString("task%1Started").arg(variantInfo->getCurrentTaskNum() + 1);
    studentInfo->setStudentData(taskId, Passed ? "true" : "false");
    updateSelectedStudentFields(QStringList{taskId});
}
    //-------------СОЗДАНИЕ ИНТЕРФЕЙСА


    //+++++++++++++ОБРАБОТКА СОБЫТИЙ

void MainWindow::showStudentSelectionDialog()
{
    qDebug() << "showStudentSelectionDialog";
    if (studentsList.isEmpty())
    {
        //while (studentsList.empty())
        {
            auto reply = QMessageBox::question(this, "Список пуст", "Список студентов пуст. Повторить попытку?",
                                               QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                sendRequest("getStudentList", false);
                qDebug() << "getStudentList return";
                return;
            }
            //QMessageBox::information(this, "getStudentListdata", "getStudentListdata!");

        qApp->quit();
        return;
        }
    }
    else
    {
        qDebug() << "showStudentSelectionDialog ended";

        emit("studentListCreate");
    }

}

void MainWindow::studentListCreated()
{
    qDebug() << "studentInputDialog:" ;
    QString filePath = "";

    StudentInputDialog* studentInputDialog = new StudentInputDialog(studentsList, nullptr, filePath);    // Создаем окно ввода данных

    if (studentInputDialog->exec() == QDialog::Accepted)    // Показываем окно модально
    {
        selectedStudentInfo = studentInputDialog->getStudent();  // Получаем объект Student из диалог
        studentSelected();
        qDebug() << "studentSelected:" ;

        qDebug() << "reqStudentSettings:" ;
        reqStudentSettings();
        //если нет подключения к серверу, то загрузить вариант из файла (переместив условие в конструктор)
    }
    else
    {
        QApplication::quit(); // Закрываем приложение
    }

    testDialog = new TestDialog();

    connect(testDialog, &TestDialog::sendStudentData, studentInfo, &StudentInfo::setStudentData);
    connect(testDialog, &TestDialog::requestSetting, this, &MainWindow::provideSetting);
    connect(this, &MainWindow::sendSetting, testDialog, &TestDialog::receiveSetting);
    connect(testDialog, &TestDialog::testFinished, this, &MainWindow::onTestFinished);
    connect(testDialog, &TestDialog::updateTestResult, this, &MainWindow::onUpdateTestResult);

    this->resize(1000, 800);  // Устанавливаем размер окна 800x600 пикселей

}


void MainWindow::startScheme()
{
    StartSchemeInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString portNumber = dialog.getPortNumber();
        QString controlPacket = dialog.getControlPacket();

        if (portNumber.isEmpty() || controlPacket.isEmpty())
        {
            QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены!");
            return;
        }

        bool ok;
        int portNum = portNumber.toInt(&ok);
        if (!ok)
        {
            QMessageBox::warning(this, "Ошибка", "Некорректный номер входа!");
            return;
        }

        try {
            scene->startScheme(portNumber, controlPacket);
            //variantInfo->getCurrentTask()->setLaunchPass(true);
            setLaunchPass(true);
        }
        catch (const std::exception& e)
        {
            QMessageBox::critical(this, "Ошибка", QString("Произошла ошибка: %1").arg(e.what()));
            //variantInfo->getCurrentTask()->setLaunchPass(false);
            setLaunchPass(false);
        }
        catch (...)
        {
            QMessageBox::critical(this, "Ошибка", "Неизвестная ошибка");
            //variantInfo->getCurrentTask()->setLaunchPass(false);
            setLaunchPass(false);
        }
    }
}


void MainWindow::provideSetting(const QString &settingName)
{
    qDebug() << "settingName " << settingName;
    QVariant value = getSettingValue(settingName);
    emit sendSetting(settingName, value);
    qDebug() << "value " << value;
}

void MainWindow::setEditMode()
{
    qDebug() << "mw setSchemeMode edit";
    //setEditPass(false);

    emit setSchemeMode(SchemeMode::Edit);
}

void MainWindow::setRunMode()
{
    qDebug() << "mw setSchemeMode run";
    emit setSchemeMode(SchemeMode::Run);
}

void MainWindow::setLockSimMode()
{
    qDebug() << "mw setSchemeMode lockSimulation";
    emit setSchemeMode(SchemeMode::LockSimulation);
}

void MainWindow::deleteSelectedItem()
{
    // Получаем выделенный элемент на сцене
    QGraphicsItem* selectedItem = scene->selectedItems().isEmpty() ? nullptr : scene->selectedItems().first();

    if (selectedItem) {
        scene->changeScheme("deleteElement", selectedItem);  // Удаляем элемент с помощью метода из DiagramScene
        propertiesTable->setCurrentObject(nullptr);
        qDebug() << "delete mw ";
    } else {
        QMessageBox::information(this, "Ошибка", "Не выбран элемент для удаления.");
    }
}

void MainWindow::createVariants()
{
    variantInfo = new VariantInfo(studentInfo->getStudentData()["variant"], false);

    while (variantInfo->getTasks().isEmpty())
    {

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Вариант не выдан",
                                      "Вариант не выдан. Запросить вариант с сервера?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            reqGetStudentData();
            variantInfo->createTasks(studentInfo->getStudentData()["variant"]);
        }
        else if (reply == QMessageBox::No)
        {
            qDebug("выход");
            QApplication::quit();
            return;
        }
    }

    int i = 0;
    foreach (TaskInfo* taskInfo, variantInfo->getTasks())
    {
        i++;
        qDebug() <<"createscheme "<< taskInfo->getTopologyName();

        QString schemeStr = "";
        QVariant value = getSettingValue("createEtalonScheme");
        QString dataName;

        qDebug() <<"value "<< value;
        if (value == true)
        {
            schemeStr = taskInfo->getDiagramScene()->getReferenceString();
            dataName = "scheme" + QString::number(i) + "Str";
            qDebug() <<"schemeStr1 "<< schemeStr;
        }
        else
        {
            dataName = "scheme" + QString::number(i) + "Str";
            qDebug() <<"dataName "<< dataName;
            schemeStr = studentInfo->getStudentData()[dataName];
            qDebug() <<"schemeStr2 "<< schemeStr;
        }


        if (taskInfo->getTopology() != nullptr) taskInfo->getDiagramScene()->createScheme(schemeStr);

        studentInfo->setStudentData(dataName, schemeStr);
        qDebug() <<"aftercreatescheme";
        connect(elementSelector, &ElementSelector::changeState, taskInfo->getDiagramScene(), &DiagramScene::setState);    // Подключаем сигнал выбора элемента к слоту для обработки
        //connect(propertiesTable, &PropertiesTable::propertiesChanged, taskInfo->getDiagramScene(), &DiagramScene::updateElementProperties);
        qDebug() <<"aftercreatescheme";
    }

    qDebug() << "changeCurrTask0:" ;

    changeCurrTask(-1, 0);
    qDebug() << "changeCurrTask:" ;


}


void MainWindow::onUpdateTestResult(QMap<QString, QString>& results)
{
    if (!studentInfo)
        return;

    // Получаем текущие данные студента (QMap<QString, QString>)
    QMap<QString, QString> data = studentInfo->getStudentData();

    // Обновляем нужные поля, если они присутствуют в results:
    if (results.contains("testCount"))
        data["testCount"] = results["testCount"];

    if (results.contains("testPassed"))
        data["testPassed"] = results["testPassed"];

    if (results.contains("bestResult"))
        data["bestResult"] = results["bestResult"];

    // Обновляем данные студента. Поскольку в классе StudentInfo метод setStudentData принимает ключ и значение,
    // проходим по обновлённой карте и вызываем метод для каждого поля:
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        studentInfo->setStudentData(it.key(), it.value());
    }

    updateSelectedStudentFields(results.keys());
}

void MainWindow::onTestFinished()
{
    if (!this->isVisible())
    {
        qDebug() << "w->show()";

        createDockWidgets();
        qDebug() << "createDockWidgets:" ;

        createActions();
        qDebug() << "createActions:" ;

        this->createVariants();
        createMenu();
        qDebug() << "createMenu:" ;

        this->show();
    }
}

void MainWindow::updateStudentData()
{
    QString dataName = "scheme" + QString::number(variantInfo->getCurrentTaskNum() + 1) + "Str";

    studentInfo->setStudentData(dataName, variantInfo->getCurrentTask()->getDiagramScene()->getCurrStr());
    qDebug() << "variantInfo->getCurrentTask()->getDiagramScene()->getCurrStr() "<<variantInfo->getCurrentTask()->getDiagramScene()->getCurrStr();

    updateSelectedStudentFields({dataName});

}

    //-------------ОБРАБОТКА СОБЫТИЙ

//-------------ИНТЕРФЕЙС



//-------------ДИАЛОГОВЫЕ ОКНА

void MainWindow::showTaskInfoDialog()
{
    VariantInfoDialog dialog(variantInfo, this);
    connect(&dialog, &VariantInfoDialog::taskChanged, variantInfo, &VariantInfo::changeCurrTask);
    connect(&dialog, &VariantInfoDialog::taskChanged, this, &MainWindow::changeCurrTask);

    dialog.exec();
}

void MainWindow::changeCurrTask(int prevTask, int selectedTask)
{
    // Отключаем сигналы и слоты перед заменой сцены
    //QSignalBlocker blocker(scene);

    if (scene != nullptr)
    {
        //disconnect(propertiesTable, &PropertiesTable::valueChanged, this, &MainWindow::updateElementProperties);
        //  disconnect(propertiesTable, &PropertiesTable::propertiesChanged, scene, &DiagramScene::updateElementProperties);

        /* disconnect(scene, &DiagramScene::itemInserted, this, &MainWindow::itemInserted);
        disconnect(scene, &DiagramScene::itemSelected, this, &MainWindow::itemSelected);
        disconnect(scene, &DiagramScene::IOPortSelected, this, &MainWindow::validateScheme);
        //disconnect(scene, &DiagramScene::itemSelected, this, &MainWindow::updatePropertiesTable);
        disconnect(scene, &DiagramScene::hidePropertiesPanel, this, &MainWindow::hidePropertiesPanel);
        disconnect(scene, &DiagramScene::showPropertiesPanel, this, &MainWindow::showPropertiesPanel);*/


        // Устанавливаем сохраненную ранее сцену
        //scene->setState("");
        scene = variantInfo->getCurrentTask()->getDiagramScene();

        connect(this, MainWindow::setSchemeMode, scene, DiagramScene::setSchemeMode);
        connect(propertiesTable, &PropertiesTable::propertiesChanged, scene, &DiagramScene::changeScheme);
        connect(scene, &DiagramScene::schemeChanged, this, &MainWindow::setEditPass);
        connect(scene, &DiagramScene::schemeChanged, this, &MainWindow::updateStudentData);
        connect(scene, &DiagramScene::pathEnd, this, &MainWindow::pathEnd);

        void ();

        updateMenuState();
        //view->setScene(scene);

        //connect(propertiesTable, &PropertiesTable::valueChanged, this, &MainWindow::updateElementProperties);
        // connect(propertiesTable, &PropertiesTable::propertiesChanged, scene, &DiagramScene::updateElementProperties);

        /*connect(scene, &DiagramScene::itemInserted, this, &MainWindow::itemInserted);
        connect(scene, &DiagramScene::itemSelected, this, &MainWindow::itemSelected);
        connect(scene, &DiagramScene::IOPortSelected, this, &MainWindow::validateScheme);
        //connect(scene, &DiagramScene::itemSelected, this, &MainWindow::updatePropertiesTable);
        connect(scene, &DiagramScene::hidePropertiesPanel, this, &MainWindow::hidePropertiesPanel);
        connect(scene, &DiagramScene::showPropertiesPanel, this, &MainWindow::showPropertiesPanel);    */
    } else
    {
        //scene->setState("");
        disconnect(this, MainWindow::setSchemeMode, scene, DiagramScene::setSchemeMode);
        disconnect(propertiesTable, &PropertiesTable::propertiesChanged, scene, &DiagramScene::changeScheme);
        disconnect(scene, &DiagramScene::schemeChanged, this, &MainWindow::setEditPass);
        disconnect(scene, &DiagramScene::schemeChanged, this, &MainWindow::updateStudentData);
        disconnect(scene, &DiagramScene::pathEnd, this, &MainWindow::pathEnd);

        scene = variantInfo->getTasks()[selectedTask]->getDiagramScene();

        connect(this, MainWindow::setSchemeMode, scene, DiagramScene::setSchemeMode);
        connect(propertiesTable, &PropertiesTable::propertiesChanged, scene, &DiagramScene::changeScheme);
        connect(scene, &DiagramScene::schemeChanged, this, &MainWindow::setEditPass);
        connect(scene, &DiagramScene::schemeChanged, this, &MainWindow::updateStudentData);
        connect(scene, &DiagramScene::pathEnd, this, &MainWindow::pathEnd);

    }
    if (scene->parent() == nullptr)
    {
        scene->setParent(this);
        //и меню ещё
    }
    view->setScene(scene);
    QRectF boundingRect = scene->itemsBoundingRect();
    qDebug() << "Bounding rect of scene:" << boundingRect;

    // Центрируем и масштабируем сцену
    view->centerOn(boundingRect.center());
}

//-------------ДИАЛОГОВЫЕ ОКНА



//+++++++++++++ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ

void MainWindow::connectToServer(const QString& host, quint16 port)
{
    qDebug("Подключение к серверу...");
    socket->connectToHost(host, port);

    if (socket->waitForConnected(3000))
    {
        sendRequest("getStudentList");
    }
    else
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу!");
    }

}

QVariant MainWindow::getSettingValue(const QString& settingName) const
{
    if (!studentSettings.contains(settingName)) {
        qWarning() << "Настройка не найдена: " << settingName;
        return QVariant();
    }

    QString value = studentSettings[settingName].first;
    QString type = studentSettings[settingName].second;

    if (type == "int")
    {
        return value.toInt();
    }
    else if (type == "bool")
    {
        return (value.toLower() == "true");
    }
    else if (type == "double")
    {
        return value.toDouble();
    }
    else
    {
        return value;  // По умолчанию строка
    }
}

//-------------ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ

