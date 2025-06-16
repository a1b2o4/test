
#include "port.h"
#include "connection.h"
#include "switchingelement.h"

int Port::maxInputPortNum = 0;
int Port::maxOutputPortNum = 0;

//Port::Port(bool isInputPort, QString _numInElement, SwitchingElement* switchingElement, QString _numInStage)
Port::Port(SwitchingElement* switchingElement, QMap<QString, QString> properties)
    : inChannel(false), switchingElement(switchingElement)
{
    setFields(properties);
}

Port::~Port()
{
    qDebug()<<"delete port " << this;//<< getFields();
    qDeleteAll(inputConnections);
    qDeleteAll(outputConnections);

    inputConnections.clear();
    outputConnections.clear();
    qDebug()<<"delete port end" ;
}

void Port::removeInputConnection(Connection* conn)
{
    inputConnections.removeAt(inputConnections.indexOf(conn));
}

void Port::removeOutputConnection(Connection* conn)
{
    outputConnections.removeAt(outputConnections.indexOf(conn));
}

QString Port::getStage()
{
    if (getSwitchingElement() == nullptr)
    {
        return " ";
    }
    else
    {
        return getSwitchingElement()->getStage();
    }
}

void Port::switchToNextOutput()
{
    qDebug()<<"switchToNextOutput ";
    if (!switchingElement || !isInputPort()) return;

    QList<Port*> outputPorts = switchingElement->getOutputPorts();
    if (outputPorts.isEmpty()) return;

    qDebug()<<"getOutputConnections().count() "<< getOutputConnections().count();
    if (getOutputConnections().count() == 0)
    {
        return;
    }
    // Получаем текущий номер выхода в элементе
    int currentIndex = getOutputConnections().last()->getEndItem()->getNumInElement().toInt();

    // Определяем следующий индекс
    int nextIndex = (currentIndex + 1) % outputPorts.size(); // Цикличный переход

    // Новый порт, на который надо переключить соединение
    Port* newOutputPort = outputPorts[nextIndex];

    foreach (Connection* conn, getOutputConnections()) {
        qDebug()<<"myStartItem1 "<<conn->getStartItem()->getFields();
        qDebug()<<"myEndItem1 "<<conn->getEndItem()->getFields();
    }


    getOutputConnections().last()->changeEndItem(newOutputPort);

    foreach (Connection* conn, getOutputConnections()) {
        qDebug()<<"myStartItem2 "<<conn->getStartItem()->getFields();
        qDebug()<<"myEndItem2 "<<conn->getEndItem()->getFields();
    }
}
