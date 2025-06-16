#include "scheme.h"
#include "closa.h"

#include <algorithm>
#include <QString>
#include <QRegularExpression>

Scheme::Scheme(Topology* topology)
    : topology(topology)
{
    if (topology != nullptr)
        portBase = topology->getPortBase();
}

QString Scheme::createCurrSchemeString()
{
    qDebug()<< "createCurrSchemeString ";
    std::sort(swEl.begin(), swEl.end(), [](SwitchingElement* a, SwitchingElement* b) {
        if (a->getStage() == b->getStage())
            return a->getNumInStage() < b->getNumInStage();
        else
            return a->getStage() < b->getStage();
    });
    qDebug()<< "sort swEl ";

    std::sort(inputPorts.begin(), inputPorts.end(), [](Port* a, Port* b) {
        return a->getNumInStage() < b->getNumInStage();
    });
    qDebug()<< "sort inputPorts ";

    std::sort(outputPorts.begin(), outputPorts.end(), [](Port* a, Port* b) {
        return a->getNumInStage() < b->getNumInStage();
    });
    qDebug()<< "sort outputPorts ";

    qDebug()<< "connections count " << connections.count();
    std::sort(connections.begin(), connections.end(), [](Connection* a, Connection* b) {
        // если первый вход - вход сети
        qDebug()<< "sort a " << a;
        qDebug()<< "sort b " << b;

        qDebug()<< "sort getStartItem " << a->getStartItem();
        qDebug()<< "sort getStartItem " << b->getStartItem();
        qDebug()<< "sort getStartItem " << a->getStartItem()->getFields();
        qDebug()<< "sort getStartItem " << b->getStartItem()->getFields();

        if (a->getStartItem()->getStage() == b->getStartItem()->getStage())
        {
            qDebug()<< "sort if ";
            return a->getStartItem()->getNumInStage() < b->getStartItem()->getNumInStage();
        }
        qDebug()<< "sort else ";
        return (a->getStartItem()->getStage() < b->getStartItem()->getStage());

    });
    qDebug()<< "sort connections ";

    QString rstr = "";

    foreach (Port* inputPort, inputPorts)
    {
        qDebug()<< "inputPort delta "<<inputPort->getNumInStage();
        rstr.append("I" + inputPort->getNumInStage()); // номер входа сети
    }

    rstr.append("|"); // отделяет входы и элементы

    foreach (SwitchingElement* swel, swEl)
    {
        rstr.append("S" + QString::number(swel->getInputCount()) + "-" + QString::number(swel->getOutputCount()));//количество входов и выходов
        rstr.append("L" + swel->getStage());                                       // номер ступени
        rstr.append("N" + swel->getNumInStage());                                  // номер элемента в ступени

        foreach (Port* eInputPort, swel->getInputPorts())
        {
            rstr.append("i" + eInputPort->getNumInElement() + "*");                // номер входа в элементе
            rstr.append(eInputPort->getNumInStage() + "*");                                         // номер входа в ступени
            rstr.append(eInputPort->getStage() + "*");                             // ступень
            rstr.append(eInputPort->getSwitchingElement()->getNumInStage());       // номер элемента для тек входа в ступени
        }
        foreach (Port* eOutputPort, swel->getOutputPorts())
        {
            rstr.append("o" + eOutputPort->getNumInElement() + "*");               // номер выхода в элементе
            rstr.append(eOutputPort->getNumInStage() + "*");                                        // номер выхода в ступени
            rstr.append(eOutputPort->getStage() + "*");                            // ступень
            rstr.append(eOutputPort->getSwitchingElement()->getNumInStage());      // номер элемента для тек выхода в ступени
        }
    }

    rstr.append("|"); // отделяет элементы и входы

    foreach (Port* outputPort, outputPorts)
    {
        rstr.append("O" + outputPort->getNumInStage()); // номер выхода сети
    }

    rstr.append("|"); // отделяет входы и соединения

    foreach (Connection* connection, connections)
    {
        rstr.append("C*");
        rstr.append((connection->getStartItem()->getStage().toInt() == -1 ? " " :  connection->getStartItem()->getStage()) + "*");     // элемент
        rstr.append(connection->getStartItem()->getNumInStage() + "*");                 // первая точка соединения
        rstr.append((connection->getEndItem()->getStage().toInt() == -1 ? " " :  connection->getEndItem()->getStage()) + "*");       // номер эл-та второй точки в след ступени
        rstr.append(connection->getEndItem()->getNumInStage());                         // вторая точка соединения (если не последняя ступень, то высчитать номер, иначе такой же
    }

    //qDebug()<< "ret rstr " << rstr;
    return rstr;

}

SwitchingElement* Scheme::addSwitchingElement(QMap<QString, QString> properties)
{
    SwitchingElement *switchingElement = new SwitchingElement(properties);
    swEl.append(switchingElement);

    return switchingElement;
}

Port* Scheme::addIOPort(QMap<QString, QString> _properties)
{
    bool inputPort = !QVariant(_properties["Вход"]).toBool();
    _properties["Вход"] = QString::number(inputPort);
    qDebug()<< "_properties[""Вход""]2" << _properties["Вход"];
    //Port* ioportItem = new Port(!inputPort, 0, nullptr);

    Port* ioportItem = new Port(nullptr, _properties);

    if (!inputPort)
    {
        qDebug() << "inputQPorts.append(ioportItem) " << ioportItem->getNumInStage()<<  " getInputCount " << getTopology()->getInputCount();
        inputPorts.append(ioportItem);
    }
    else
    {
        outputPorts.append(ioportItem);
    }

    return ioportItem;
}

Connection* Scheme::addConnection(Port *startItem, Port *endItem, bool inConnection)
{
    Connection *connItem = new Connection(startItem, endItem);

    if (!inConnection)
    {
        connect(connItem, &Connection::isDeleted, this, &Scheme::removeConnection);
        connections.append(connItem);
    }

    return connItem;
}


QList<QString> Scheme::checkScheme()
{
    QList<QString> errors;

    qDebug() << "1 ";
    // Создаём копии схем, чтобы не изменять исходные данные
    if (topology == nullptr)
    {
        qDebug() << "topology == nullptr ";
        return errors;
    }
    QString refScheme = topology->getReferenceString();
    qDebug() << "2 ";
    QString curScheme = createCurrSchemeString();

    //qDebug() << "refScheme " << refScheme;
    //qDebug() << "curScheme " << curScheme;

    //if (curScheme == refScheme) return errors;


    QStringList curSchemeElements = curScheme.split("|");//, Qt::SkipEmptyParts);
    qDebug() << "curSchemeElements " << curSchemeElements;
    // Разделяем строки на компоненты
    QStringList curInputs = splitWithDelimiters(curSchemeElements[0], "I");
    qDebug() << "curInputs " << curInputs;

    QStringList curOutputs = splitWithDelimiters(curSchemeElements[2], "O");
    qDebug() << "curOutputs " << curOutputs;

    QStringList curElements = splitWithDelimiters(curSchemeElements[1], "S");
    qDebug() << "curElements " << curElements;

    QStringList curConnections = splitWithDelimiters(curSchemeElements[3], "C");
    qDebug() << "curConnections " << curConnections;

    // Проверяем входы
    for (const QString& input : curInputs)
    {
        //qDebug() << "refScheme " << refScheme;
        qDebug() << "input " << input;
        if (refScheme.contains(input))
        {
            refScheme.replace(input, "");  // Убираем найденный элемент из строки
        } else {
            errors.append("Отсутствует вход: " + input);
        }
    }

    // Проверяем выходы
    for (const QString& output : curOutputs)
    {
        //qDebug() << "refScheme " << refScheme;
        qDebug() << "input " << output;
        if (refScheme.contains(output))
        {
            refScheme.replace(output, "");  // Убираем найденный элемент из строки
        } else {
            errors.append("Отсутствует выход: " + output);
        }
    }

    // Проверяем элементы
    for (const QString& element : curElements)
    {
        //qDebug() << "refScheme " << refScheme;
        qDebug() << "element " << element;
        if (refScheme.contains(element))
        {
            refScheme.replace(element, "");  // Убираем найденный элемент из строки
        } else {
            errors.append("Отсутствует элемент: " + element);
        }
    }

    // Проверяем соединения
    for (const QString& connection : curConnections)
    {
        //qDebug() << "refScheme " << refScheme;
        qDebug() << "connection " << connection;
        if (refScheme.contains(connection))
        {
            refScheme.replace(connection, "");  // Убираем найденное соединение из строки
        } else {
            errors.append("Неверное соединение: " + connection);
        }
    }

    // Проверяем на остаточные элементы в эталонной строке
    if (refScheme.contains("I")) {
        errors.append("Отсутствуют входы в схеме.");
    }
    if (refScheme.contains("O")) {
        errors.append("Отсутствуют выходы в схеме.");
    }
    if (refScheme.contains("S")) {
        errors.append("Отсутствуют элементы в схеме.");
    }
    if (refScheme.contains("C")) {
        errors.append("Отсутствуют соединения в схеме.");
    }

    return errors;
}

QStringList Scheme::splitWithDelimiters(const QString& str, const QString& delimiters)
{
    QRegularExpression regex("[" + delimiters + "][^" + delimiters + "]*");
    QStringList parts;

    QRegularExpressionMatchIterator it = regex.globalMatch(str);
    while (it.hasNext()) {
        parts.append(it.next().captured(0));  // Добавляем всю найденную подстроку
    }

    return parts;
}

void Scheme::getTokenPath(Port* startPort, QString& controlPacket, bool* isBlock)
{
    if (dynamic_cast<Closa*>(topology)) controlPacket = "-" + controlPacket;


    pathConnections.clear();
    if (!startPort) return;

    Connection* connection;
    Port* currentPort = startPort;
    pathPorts.append(currentPort);
    qDebug() << "occupiedPorts.insert: " << currentPort->getNumInStage();

    occupiedPorts.insert(currentPort);

    connection = getNextConnection(currentPort);    // Первая связь между входом сети и первой ступенью
    if (!connection)
    {
        qDebug() << "!connection: " << !connection;
        return;
    }
    qDebug() << "connectiongetStartItem: " << connection->getStartItem()->getNumInStage();
    qDebug() << "connectiongetEndItem: " << connection->getEndItem()->getNumInStage();

    currentPort = getNextPort(connection, isBlock);
    pathPorts.append(currentPort);

    if (*isBlock) return;
    qDebug() << "currentPort next: " << currentPort->getNumInStage();

    for (int i = 0; i < controlPacket.size(); ++i)    // Проход по ступеням сети
    {
        //pathPorts.append(currentPort);

        processStage(currentPort, controlPacket, isBlock);
        qDebug()<<"processStage 1 ";
        if (*isBlock)
        {
            qDebug()<<"isblock 1 ";
            break;
        }
    }

    qDebug() << "occupiedPorts:";
    for (Port* port : occupiedPorts) {
        if (port) {
            qDebug() << "Port number getNumInStage:" << port->getNumInStage(); // Используйте ваш метод получения номера порта
            qDebug() << "Port number getNumInElement:" << port->getNumInElement(); // Используйте ваш метод получения номера порта
            qDebug() << "Port number getStage:" << port->getStage(); // Используйте ваш метод получения номера порта
        }
    }

    qDebug() << "pathConnections:";
    for (Connection* conn : pathConnections) {
        if (conn && conn->getStartItem() && conn->getEndItem()) {
            qDebug() << "Connection from Port"
                     << conn->getStartItem()->getNumInStage() << " "
                     << conn->getStartItem()->getStage()
                     << "to Port"
                     << conn->getEndItem()->getNumInStage() << " "
                     << conn->getEndItem()->getStage();
        }
    }

}

Connection* Scheme::getNextConnection(Port* port)
{
    qDebug()<< "getNextConnection port "<< port->getNumInStage();
    if (port && !port->getOutputConnections().isEmpty())
    {
        Connection* currConn = port->getOutputConnections().first();
        currConn->setInChannel(true);
        pathConnections.append(currConn);

        return currConn;
    }
    return nullptr;
}

Port* Scheme::getNextPort(Connection* conn, bool* isBlock)
{
    //qDebug() << "conn->getEndItem() " << conn->getEndItem()->getFields();
    if (conn && conn->getEndItem())
    {
        Port* currentPort = conn->getEndItem();
        if (occupiedPorts.contains(currentPort))// Проверяем блокировку
        {
            *isBlock = true;
            qDebug() << "isBlock " << isBlock;
            return currentPort;  // Остановка при блокировке
        }
        occupiedPorts.insert(currentPort);

        return currentPort;
    }
    qDebug() << "getNextPort nullptr";
    return nullptr;
}

Port* Scheme::getOutPortInElement(Port* currentPort, QString controlPacket, bool* isBlock)
{
    currentPort = topology->getOutPortInElement(currentPort, controlPacket, occupiedPorts);

    qDebug() << "currentPort getOutPortInElement " << !currentPort;

    if (occupiedPorts.contains(currentPort))// Проверяем блокировку
    {
        *isBlock = true;
        return currentPort;  // Остановка при блокировке
    }
    occupiedPorts.insert(currentPort);

    return currentPort;
}

void Scheme::processStage(Port*& currentPort, QString controlPacket, bool* isBlock)// Обрабатывает ступень сети
{
    //if (!currentPort) return;
    Connection* tmpConnection = getNextConnection(currentPort);    // Связь внутри элемента

   // if (!internalConnection) return;

    currentPort = getOutPortInElement(currentPort, controlPacket, isBlock);
    pathPorts.append(currentPort);

    qDebug() << "currentPort " << currentPort->getFields();
    if (currentPort->getOutputConnections().isEmpty()) {
        qDebug() << "currentPort connection empty";
        *isBlock = true;
        return;
    }
    qDebug() << "currentPort connection " << currentPort->getOutputConnections()[0]->getEndItem()->getFields();

    if (currentPort->getNumInElement() != tmpConnection->getEndItem()->getNumInElement())
    {
        qDebug() << "changeEndItem " ;

        tmpConnection->changeEndItem(currentPort);
    }
    // переключить соединение
    if (*isBlock)
    {
        qDebug() << "*isBlock " << *isBlock;

        return;
    }
    qDebug() << "1 " ;

    tmpConnection = getNextConnection(currentPort);    // Связь между ступенями
    //if (!interStageConnection) return;
    qDebug() << "getNextConnection "<<!tmpConnection ;
    if (!tmpConnection) {
        qDebug() << "a3 " ;
        return;
    }

    currentPort = getNextPort(tmpConnection, isBlock);
    pathPorts.append(currentPort);

    qDebug() << "currentPort " << currentPort->getFields();
    if (*isBlock) return;

}

Port* Scheme::findPortByNumber(QString portNumber)
{
    for (Port* port : inputPorts)  // `allPorts` — список всех портов схемы
    {
        if (port->getNumInStage() == portNumber)  // Предполагается, что у Port есть метод getNumber()
        {
            return port;
        }
    }
    return nullptr;
}

void Scheme::changedSchemeMode(SchemeMode _mode)
{
    occupiedPorts.clear();
    pathConnections.clear();
    pathPorts.clear();
}

// Метод для удаления коммутатора
void Scheme::removeSwitchingElement(SwitchingElement* switchingElement)
{
    // Найдем и удалим коммутатор из схемы
    foreach (Connection* conn, switchingElement->getConnections())
    {
        removeConnection(conn);
    }
    foreach (Port* port, switchingElement->getInputPorts())
    {
        removePort(port);
    }
    foreach (Port* port, switchingElement->getOutputPorts())
    {
        removePort(port);
    }

    swEl.removeOne(switchingElement);
    //delete switchingElement; // Освобождаем память
}

// Метод для удаления порта
void Scheme::removePort(Port* port)
{
    qDebug()<<"connections " << connections.count();
    foreach (Connection* conn, connections)
    {
        qDebug()<<"connections " << conn;
    }

    foreach (Connection* conn, port->getInputConnections())
    {
        qDebug()<<"connections port " << conn;
        connections.removeAll(conn);
    }
    foreach (Connection* conn, port->getOutputConnections())
    {
        qDebug()<<"connections port 2 " << conn;
        connections.removeAll(conn);
    }
    qDebug()<<"connections " << connections.count();

    // Найдем и удалим порт из схемы
    inputPorts.removeOne(port);
    qDebug()<<"inputPorts " << inputPorts.count();

    outputPorts.removeOne(port);
    qDebug()<<"outputPorts " << outputPorts.count();

    //delete port; // Освобождаем память
}

void Scheme::removeConnection(Connection* connection)
{
    connections.removeOne(connection);
    //delete connection;
}
