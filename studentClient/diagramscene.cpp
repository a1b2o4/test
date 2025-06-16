#include "diagramscene.h"

#include "diagramitem.h"
#include "qswitchingelement.h"
#include "qioport.h"
#include "qport.h"
#include "qconnection.h"
#include "mainwindow.h"
#include "scheme.h"
#include "propertiestable.h"

#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>
#include <QGraphicsObject>
#include <iostream>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QGraphicsEllipseItem>

QPort* DiagramScene::getQPorttoPort(Port* port)
{ return qports[port]; }

QIOPort* DiagramScene::getInQIOPorttoPort(Port* port)
{ return inputQPorts[port]; }

QIOPort* DiagramScene::getOutQIOPorttoPort(Port* port)
{ return outputQPorts[port]; }

void DiagramScene::addConnectionToQConnection(Connection* conn, QConnection* qconn)
{ qconnections.insert(conn, qconn); }

QString DiagramScene::getCurrStr() { return scheme->createCurrSchemeString(); }


QString DiagramScene::getReferenceString()
{
    qDebug() <<"scheme->getTopology()->getReferenceString() ";//<< scheme->getTopology()->getReferenceString();

    if (!scheme) return "";
    if (!scheme->getTopology()) return "";


    return scheme->getTopology()->getReferenceString();
}

DiagramScene::DiagramScene(Topology* topology, QMenu *itemMenu, QObject *parent)
    : QGraphicsScene(parent)
{
    myItemMenu = itemMenu;
    myMode = MoveItem;
    myItemColor = Qt::white;
    myTextColor = Qt::black;
    myLineColor = Qt::black;

    scheme = new Scheme(topology);
    mode = SchemeMode::Edit;

    connect(this, DiagramScene::changeSchemeMode, scheme, Scheme::changedSchemeMode);
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    qDebug()<<"press mouse mousePressEvent";
    if (mouseEvent->button() != Qt::LeftButton)
    {
        qDebug()<<"return mousePressEvent";
        return;
    }

    QGraphicsItem *qGraphicsItemitem = itemAt(mouseEvent->scenePos(), QTransform());
    DiagramItem* item = dynamic_cast<DiagramItem*>(qGraphicsItemitem);
    qDebug()<<state;

    if (!item)
    {
        qDebug()<<"change mousePressEvent";
        item = changeScheme("addElement");
        if (item)
        {
            item->setPos(mouseEvent->scenePos());
        }
    }

    if (item)
    {
        //qDebug() << item->getFields();
        qDebug() << "mode " << (mode == SchemeMode::buildPath);

        if ((mode == SchemeMode::buildPath) && (dynamic_cast<QPort*>(item)))
        {
            pathBuildPorts.append(dynamic_cast<QPort*>(item)->getPort());
            //qDebug() << "check pathBuildPorts" << pathBuildPorts[pathBuildPorts.length()-1]->getFields();

            QMap<QString, QString> fields = dynamic_cast<QPort*>(item)->getPort()->getFields();
            qDebug() << "check end" << fields;
            if ((fields["Вход"] == "1") && (fields["Номер в элементе"] == " "))
            {
                qDebug() << "end";
                emit pathEnd();
            }
        }

        emit item->selectItem(item);  // Вызов сигнала с аргументом
    }

    QGraphicsScene::mousePressEvent(mouseEvent);
}

void DiagramScene::setSchemeMode(SchemeMode _mode, bool changeMode)
{
    pathBuildPorts.clear();

    if (changeMode)
        mode = _mode;
    else
        _mode = mode;

    if (_mode == SchemeMode::Edit)
    {
        qDebug() << "ds setSchemeMode Edit" ;//<< _mode;
    }
    if (_mode == SchemeMode::Run)
    {
        qDebug() << "ds setSchemeMode Run" ;//<< _mode;
    }
    if (_mode == SchemeMode::LockSimulation)
    {
        qDebug() << "ds setSchemeMode LockSimulation" ;//<< _mode;
    }
    if (_mode == SchemeMode::buildPath)
    {
        qDebug() << "ds setSchemeMode buildPath" ;//<< _mode;
    }

    emit changeSchemeMode(_mode);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void DiagramScene::addPortForConnectoin(QPort* port)
{
    qportsForConnection.append(port);
    if (qportsForConnection.count() == 2 && state == "Соединение")
    {
        QConnection *connection =addConnection(qportsForConnection[0], qportsForConnection[1], false);
        //connection->setColor(myLineColor);
        //connection->setZValue(1000.0);
        //addItem(connection);
        connection->updatePosition();

        qportsForConnection.clear();

    }
}

void DiagramScene::setState(QString _state)
{
    state = _state;
    qportsForConnection.clear();
}

void DiagramScene::createScheme(QString refStr)
{
    Topology* _topology = scheme->getTopology();
    if (!_topology) return;
    if (refStr.isEmpty()) return;

    // || _topology->getReferenceString().isEmpty()) return;
    //QString refStr = _topology->getReferenceString();

    QHash<QString, QPort*> elements;
    QStringList schemeElements = refStr.split("|");

    QStringList inputPorts = schemeElements[0].split("I", Qt::SkipEmptyParts);
    QStringList outputPorts = schemeElements[2].split("O", Qt::SkipEmptyParts);
    QStringList swEls = schemeElements[1].split("S", Qt::SkipEmptyParts);

    // --- Основные параметры ---
    const int stageSpacingX = 150;  // Расстояние между ступенями
    const int itemSpacingY = 50;    // Расстояние между элементами
    const int offsetX = 100;        // Одинаковый отступ для входов и выходов
    const int firstStageX = 100;    // X координата первой ступени

    // --- Определяем количество ступеней ---
    int lastStage = 0;
    int minY = INT_MAX;  // Минимальная Y-координата (определяет верх схемы)

    for (const QString &element : swEls)
    {
        QMap<QString, QString> elementProperties;
        QList<QMap<QString, QString>> inputs, outputs;
        parseElementData(element, elementProperties, inputs, outputs);
        int stage = elementProperties.value("Ступень", "0").toInt();
        lastStage = qMax(lastStage, stage);
    }
    int lastStageX = firstStageX + lastStage * stageSpacingX;

    // --- Определение верхнего края схемы (minY) ---
    QMap<int, int> switchStageCount;
    for (const QString &element : swEls)
    {
        QMap<QString, QString> elementProperties;
        QList<QMap<QString, QString>> inputs, outputs;
        parseElementData(element, elementProperties, inputs, outputs);

        int stage = elementProperties.value("Ступень", "0").toInt();
        int positionInStage = switchStageCount[stage]++;

        int numInputPorts = inputs.size();
        int numOutputPorts = outputs.size();
        int height = 100 + std::max(numInputPorts, numOutputPorts) * 20;

        QSwitchingElement* qSwitchingElement = addSwitchingElement(elementProperties);
        //QSwitchingElement* qSwitchingElement = dynamic_cast<QSwitchingElement*>(changeScheme("addElement", nullptr, elementProperties));

        QPointF pos(firstStageX + stage * stageSpacingX, positionInStage * height);
        qDebug()<<"pos " <<pos;
        qSwitchingElement->setPos(pos);
        addItem(qSwitchingElement);

        // Запоминаем самый верхний элемент
        minY = qMin(minY, (int)pos.y());

        // --- Добавляем входные порты элемента в `elements` ---
        QList<QPort*> inputQports = qSwitchingElement->getInputQPorts();
        qDebug() << "insert(inputQports " << inputQports.count();
        for (int nowPort = 0; nowPort < inputQports.count(); nowPort++)
        {
            qDebug()<<"inputs[nowPort] "<<inputs[nowPort];
            inputQports[nowPort]->getPort()->setFields(inputs[nowPort]);
            QString hash = Scheme::generateHash("1-" + inputQports[nowPort]->getPort()->getStage() + "-" + inputQports[nowPort]->getPort()->getNumInStage());
            elements.insert(hash, inputQports[nowPort]);
            qDebug() << "insert(hashStr1 " << hash;
            qDebug() << "insert(hashStr1 " << "1-" + inputQports[nowPort]->getPort()->getStage() + "-" + inputQports[nowPort]->getPort()->getNumInStage();
        }

        // --- Добавляем выходные порты элемента в `elements` ---
        QList<QPort*> outputQports = qSwitchingElement->getOutputQPorts();
        for (int nowPort = 0; nowPort < outputQports.count(); nowPort++)
        {
            outputQports[nowPort]->getPort()->setFields(outputs[nowPort]);
            QString hash = Scheme::generateHash("0-" + outputQports[nowPort]->getPort()->getStage() + "-" + outputQports[nowPort]->getPort()->getNumInStage());
            elements.insert(hash, outputQports[nowPort]);
            qDebug() << "insert(hashStr2 " << hash;
            qDebug() << "insert(hashStr2 " << "0-" + outputQports[nowPort]->getPort()->getStage() + "-" + outputQports[nowPort]->getPort()->getNumInStage();
        }
    }

    // --- Расположение входов сети ---
    int i = 0;
    for (const QString &port : inputPorts)
    {
        QMap<QString, QString> _properties;
        _properties.insert("Номер в ступени", port);

        QIOPort* newqioport = addIOPort(true, _properties);
        //QIOPort* newqioport = dynamic_cast<QIOPort*>(changeScheme("addElement", nullptr, elementProperties));

        // Входы теперь находятся **на том же уровне**, что и вершина первой ступени
        newqioport->setPos(QPointF(firstStageX - offsetX - 40, minY + i * itemSpacingY - 10));
        addItem(newqioport);

        QString hashStr = Scheme::generateHash("0-" + newqioport->getQPort()->getPort()->getStage() + "-" + newqioport->getQPort()->getPort()->getNumInStage());
        elements.insert(hashStr, newqioport->getQPort());

        i++;
    }

    // --- Расположение выходов сети ---
    i = 0;
    for (const QString &port : outputPorts)
    {
        QMap<QString, QString> _properties;
        _properties.insert("Номер в ступени", port);
        QIOPort* newqioport = addIOPort(false, _properties);
        //QIOPort* newqioport = dynamic_cast<QIOPort*>(changeScheme("addElement", nullptr, elementProperties));

        // Выходы теперь на таком же уровне, как и входы
        newqioport->setPos(QPointF(lastStageX + offsetX, minY + i * itemSpacingY - 10));
        addItem(newqioport);

        QString hashStr = Scheme::generateHash("1-" + newqioport->getQPort()->getPort()->getStage() + "-" + newqioport->getQPort()->getPort()->getNumInStage());
        elements.insert(hashStr, newqioport->getQPort());
        i++;
    }

    // --- Соединения ---
    qDebug() << "elements";
    foreach (QString element, elements.keys())
    {
        qDebug() << "элемент:" << element<<" hash " << elements[element]->getFields();
        qDebug() << QString::number((elements[element]->getPort()->isInputPort() ? 1 : 0 )) + "-" + elements[element]->getPort()->getStage() + "-" + elements[element]->getPort()->getNumInStage();
    }

    QStringList connectionsStr = schemeElements[3].split("C", Qt::SkipEmptyParts);
    for (const QString &connectionStr : connectionsStr)
    {
        QStringList parts = connectionStr.split('*', Qt::SkipEmptyParts);
        if (parts.size() < 4) continue;  // Пропускаем некорректные строки

        QString hashStart = Scheme::generateHash("0-" + parts[0] + "-" + parts[1]);
        QString hashEnd = Scheme::generateHash("1-" + parts[2] + "-" + parts[3]);



        // Проверяем, есть ли такие хэши в elements
        if (!elements.contains(hashStart)) {
            qDebug() << "Не найден элемент для hashStart:" << hashStart;
            qDebug() << "элемент:" << "0-" + parts[0] + "-" + parts[1];
            qDebug() << "connectionStr:" << connectionStr;
            continue;
        }
        if (!elements.contains(hashEnd)) {
            qDebug() << "Не найден элемент для hashEnd:" << hashEnd;
            qDebug() << "элемент:" << "1-" + parts[2] + "-" + parts[3];
            qDebug() << "connectionStr:" << connectionStr;
            continue;
        }

        // Создаём соединение только если оба элемента существуют
        QConnection* conn = addConnection(elements.value(hashStart), elements.value(hashEnd), false);
        //addItem(conn);
    }
}




QSwitchingElement* DiagramScene::addSwitchingElement(QMap<QString, QString> properties)
{
    //item->setPos(mouseEvent->scenePos());
    SwitchingElement *swEl = scheme->addSwitchingElement(properties);

    //QSwitchingElement *switchingItem = new QSwitchingElement(myItemMenu, 2, 2, nullptr, this);
    QSwitchingElement *qswitchingItem = new QSwitchingElement(myItemMenu, swEl, nullptr, this);

    foreach (QPort* qport, qswitchingItem->getInputQPorts())
    {
        //qports.append(qport);
        qports.insert(qport->getPort(), qport);
    }

    foreach (QPort* qport, qswitchingItem->getOutputQPorts())
    {
        //qports.append(qport);
        qports.insert(qport->getPort(), qport);
    }

    //qswitchingElements.append(qswitchingItem);
    qswitchingElements.insert(swEl, qswitchingItem);

    return qswitchingItem;
}

QIOPort* DiagramScene::addIOPort(bool inputPort, QMap<QString, QString> _properties)
{
    //item->setPos(mouseEvent->scenePos());
    _properties["Вход"] = QString::number(inputPort);
    qDebug() << "_properties[""Вход""]" << _properties["Вход"];

    Port* newPort = scheme->addIOPort(_properties);

    //QIOPort *ioportItem = new QIOPort(-10, -10, 50, 20, inputPort, this);
    QIOPort *ioportItem = new QIOPort(-10, -10, 50, 20, newPort, this);

    if (inputPort)
    {
        //inputQPorts.append(ioportItem);
        inputQPorts.insert(ioportItem->getQPort()->getPort(), ioportItem);
    } else
    {
        //outputQPorts.append(ioportItem);
        outputQPorts.insert(ioportItem->getQPort()->getPort(), ioportItem);
    }

    return ioportItem;
}

QConnection* DiagramScene::addConnection(QPort* firstQPort, QPort* secondQPort, bool inConnection)
{
    Connection* connection = scheme->addConnection(firstQPort->getPort(), secondQPort->getPort(), inConnection);
    QConnection *connItem = new QConnection(firstQPort, secondQPort, connection, this);

    addConnectionToQConnection(connection, connItem);

    addItem(connItem);

    return connItem;
}


void DiagramScene::parseElementData(const QString& data,
                                    QMap<QString, QString>& elementProperties,
                                    QList<QMap<QString, QString>>& inputs,
                                    QList<QMap<QString, QString>>& outputs)
{
    QStringList parts = data.split('i');  // Разделяем строку по 'I'

    if (parts.isEmpty()) {
        qDebug() << "Ошибка: строка не содержит данных!";
        return;
    }

    QString elementPart = parts[0];    // Первая часть (до первого 'I') — параметры элемента
    QStringList elementParts = elementPart.split('L');

    if (elementParts.size() < 2) {
        qDebug() << "Ошибка: неверный формат элемента!";
        return;
    }

    QStringList sizes = elementParts[0].split('-');

    if (sizes.size() < 2) {
        qDebug() << "Ошибка: не удалось найти количество входов/выходов!";
        return;
    }

    elementProperties["Количество входов"] = sizes[0];
    elementProperties["Количество выходов"] = sizes[1];

    QStringList levelParts = elementParts[1].split('N');

    if (levelParts.size() < 2) {
        qDebug() << "Ошибка: неверный формат ступени и номера!";
        return;
    }

    elementProperties["Ступень"] = levelParts[0];
    elementProperties["Номер в ступени"] = levelParts[1];


    for (int i = 1; i < parts.size(); ++i)    // Обрабатываем входы и выходы
    {
        QString ioData = parts[i];

        // Разбиваем входы и выходы по 'O', но оставляем первый элемент (он — вход)
        QStringList ioParts = ioData.split('o', Qt::KeepEmptyParts);
        bool firstIsInput = true;

        for (const QString &ioPart : ioParts) {
            if (ioPart.isEmpty()) continue;  // Пропускаем пустые части

            QStringList inputParts = ioPart.split('*');

            if (inputParts.size() < 4) {
                qDebug() << "Ошибка: некорректные данные" << ioPart;
                continue;
            }

            QMap<QString, QString> ioMap;
            ioMap["Номер в элементе"] = inputParts[0];
            ioMap["Номер в ступени"] = inputParts[1];
            ioMap["Ступень"] = inputParts[2];
            ioMap["Номер элемента"] = inputParts[3];

            if (firstIsInput) {
                inputs.append(ioMap);
            } else {
                outputs.append(ioMap);
            }

            firstIsInput = false;  // После первого элемента всё остальное — выходы
        }
    }

    // Отладочный вывод
    qDebug() << "'Элементы:";
    for (const auto& elementProperty : elementProperties) qDebug() << elementProperty;

    qDebug() << "Входы:";
    for (const auto& input : inputs) qDebug() << input;

    qDebug() << "Выходы:";
    for (const auto& output : outputs) qDebug() << output;
}

QList<QString> DiagramScene::checkScheme()
{
    return getScheme()->checkScheme();
}

void DiagramScene::generatePathCoordinates(QList<Connection*>& pathConnections, QList<QPointF>& pathCoordinates)
{
    // Очищаем предыдущие координаты
    pathCoordinates.clear();

    // Проходим по каждому соединению в пути
    foreach (Connection* connection, pathConnections)
    {
        QConnection* qConnection = qconnections.value(connection); // Получаем QConnection для текущего Connection
        if (!qConnection) continue; // Пропускаем, если QConnection не найден

        qConnection->updateMyItems();
        qConnection->setColor(Qt::red);

        // Получаем координаты начального и конечного портов
        QPointF startPortCenter = qConnection->getQStartItem()->scenePos() + qConnection->getQStartItem()->boundingRect().center();
        QPointF endPortCenter = qConnection->getQEndItem()->scenePos() + qConnection->getQEndItem()->boundingRect().center();

        // Проверяем, соединение внутри одного элемента или нет
        if (qConnection->getQStartItem()->parentItem() != qConnection->getQEndItem()->parentItem())
        {
            // Внешнее соединение (состоит из 3 частей)
            QPointF midStart = startPortCenter + QPointF(20, 0);
            QPointF midEnd = endPortCenter + QPointF(-20, 0);

            // Добавляем все точки в путь
            pathCoordinates.append(startPortCenter);
            pathCoordinates.append(midStart);
            pathCoordinates.append(midEnd);
            pathCoordinates.append(endPortCenter);
        }
        else
        {
            // Внутреннее соединение (прямая линия)
            pathCoordinates.append(startPortCenter);
            pathCoordinates.append(endPortCenter);
        }
    }
}


void DiagramScene::startTokenAnimation(QList<QPointF>& pathCoordinates, bool isBlock)
{
    if (pathCoordinates.isEmpty()) return; // Проверка на пустой путь

    // Создание токена
    QGraphicsEllipseItem* tokenItem = new QGraphicsEllipseItem(-5, -5, 10, 10);

    tokenItem->setBrush(Qt::blue);
    addItem(tokenItem);

    connect(this, &DiagramScene::changeSchemeMode,
            [this, tokenItem]() mutable
            {
                qDebug() << "delete token";
                removeItem(tokenItem); // Удаляем объект из сцены
                delete tokenItem;  // Освобождаем память
                tokenItem = nullptr;  // Защита от повторного удаления
    });

    // Устанавливаем начальную позицию токена
    tokenItem->setPos(pathCoordinates.first());

    // Рассчитываем общую длину пути
    double totalLength = 0.0;
    QVector<double> segmentLengths;

    for (int i = 1; i < pathCoordinates.size(); ++i) {
        double segmentLength = QLineF(pathCoordinates[i - 1], pathCoordinates[i]).length();
        segmentLengths.append(segmentLength);
        totalLength += segmentLength;
    }

    if (totalLength == 0) return; // Предотвращаем деление на 0

    // Создаём анимацию
    QVariantAnimation* animation = new QVariantAnimation(this);
    animation->setDuration(1000); // Общая длительность, например, 1000 мс
    animation->setEasingCurve(QEasingCurve::Linear); // Линейное движение

    // Устанавливаем ключевые кадры
    double accumulatedTime = 0.0;
    for (int i = 0; i < segmentLengths.size(); ++i) {
        accumulatedTime += segmentLengths[i] / totalLength; // Доля времени для каждого участка
        animation->setKeyValueAt(accumulatedTime, pathCoordinates[i + 1]);
    }

    // Гарантируем, что токен остаётся в конечной позиции
    animation->setKeyValueAt(1.0, pathCoordinates.last());

    // Обновление позиции токена
    connect(animation, &QVariantAnimation::valueChanged, [=](const QVariant& value) {
        tokenItem->setPos(value.toPointF());
    });

    // Останавливаем анимацию в конце
    connect(animation, &QVariantAnimation::finished, [=]() {
        tokenItem->setPos(pathCoordinates.last());
        if (isBlock)
        {
            tokenItem->setBrush(QColorConstants::Svg::orange);
        }
    });
    // Удаляем токен после завершения анимации
    /*connect(animation, &QVariantAnimation::finished, [=]() {
        removeItem(tokenItem); // Удаляем с сцены
        delete tokenItem;      // Удаляем объект
    });*/
    // Запуск анимации
    animation->start();
}

void DiagramScene::startScheme(QString portNumber, QString controlPacket)
{
    Port* startPort = scheme->findPortByNumber(portNumber);
    if (!startPort)
    {
        QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Ошибка", "Вход с таким номером не найден!");
        return;
    }

    if (!startPort) return;

    if (!scheme->getTopology()->checkControlPacket(controlPacket))
    {
        QMessageBox::warning(qobject_cast<QWidget*>(parent()), "Ошибка", "Ошибка управляющего пакета!");
        return;
    }

    bool isBlock;
    scheme->getTokenPath(startPort, controlPacket, &isBlock);    // Получить пути соединений

    QList<QPointF> pathCoordinates;
    QList<Connection*> pathConnections = scheme->getPathConnections();
    generatePathCoordinates(pathConnections, pathCoordinates);
    startTokenAnimation(pathCoordinates, isBlock);                              // Шаг 3: Запустить анимацию
}




DiagramItem* DiagramScene::changeScheme(QString operation, QGraphicsItem* item, QMap<QString, QString> properties)
{
    bool sendEmit = false;
    DiagramItem* res = nullptr;
    if (mode != SchemeMode::Edit)
    {
        qDebug()<<"mode != SchemeMode::Edit";
        return nullptr;
    }
    qDebug()<<"mode == SchemeMode::Edit";


    if (operation == "addElement")
    {
        res = addElement(state);
        if (res) sendEmit = true;
    }
    else
    if (operation == "deleteElement")
    {
        if (item) sendEmit = true;
        deleteElement(item);
    }
    else
    if (operation == "changeElement")  // изменение из
    {
        updateElement(properties, dynamic_cast<DiagramItem*>(item));
        if (item) sendEmit = true;
    }

    if ((operation != "") && (sendEmit)) emit schemeChanged();
    qDebug()<<"operation == "<< operation;

    return res;
}

void DiagramScene::updateElement(QMap<QString, QString> properties, DiagramItem *currentObject)
{
    if (currentObject != nullptr)
    {
        currentObject->setFields(properties);
    }

    currentObject->update();
}

DiagramItem* DiagramScene::addElement(QString _state)
{
    DiagramItem* item = nullptr;

    if (_state.contains("БКЭ"))
    {
        qDebug()<<"БКЭ";

        QSwitchingElement *element = addSwitchingElement();

        addItem(element);
        item = dynamic_cast<DiagramItem*>(element);
        qDebug()<<"add sw";
    }
    else if (_state.contains("Соединение"))
    {
        qDebug()<<"click on scene";
    }
    else if (_state.contains("Вход"))
    {
        QIOPort *inputPort = addIOPort(true);
        addItem(inputPort);
        item = dynamic_cast<DiagramItem*>(inputPort);
    }
    else if (_state.contains("Выход"))
    {
        QIOPort *outputPort = addIOPort(false);

        addItem(outputPort);
        item = dynamic_cast<DiagramItem*>(outputPort);
    }

    return item;
}


void DiagramScene::deleteElement(QGraphicsItem* item)
{
    if (!item) return;

    // Проверяем, является ли элемент портом
    if (QPort* qport = dynamic_cast<QPort*>(item))
    {
        qports.remove(qport->getPort());
        delete qport;
        return;
    }

    if (QIOPort* qioport = dynamic_cast<QIOPort*>(item))
    {
        qDebug() << "delete qioport in scene";

        if (inputQPorts.contains(qioport->getQPort()->getPort()))
        {
            inputQPorts.remove(qioport->getQPort()->getPort());
        }
        else if (outputQPorts.contains(qioport->getQPort()->getPort()))
        {
            outputQPorts.remove(qioport->getQPort()->getPort());
        }
        scheme->removePort(qioport->getQPort()->getPort());
        delete qioport;
        return;
    }

    // Проверяем, является ли элемент соединением
    if (QConnection* qconn = dynamic_cast<QConnection*>(item))
    {
        qDebug() << "delete connection in scene";
        qconnections.remove(qconn->getConnection());
        scheme->removeConnection(qconn->getConnection());
        delete qconn;
        return;
    }

    // Проверяем, является ли элемент коммутатором
    if (QSwitchingElement* qsw = dynamic_cast<QSwitchingElement*>(item))
    {
        qDebug() << "delete qswitchingElements in scene";
        qswitchingElements.remove(qsw->getSwitchingElement());
        scheme->removeSwitchingElement(qsw->getSwitchingElement());
        delete qsw;
        return;
    }

    // Если это просто графический элемент (например, текст)
    delete item;
}
