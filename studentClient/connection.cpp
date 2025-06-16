#include <QList>

#include "connection.h"
#include "port.h"
#include "switchingelement.h"

Connection::Connection(Port *startItem, Port *endItem, bool _innerConnection)
    : inChannel(false), innerConnection(_innerConnection)
{
    SwitchingElement* startItemParent = startItem->getSwitchingElement();
    SwitchingElement* endItemParent = endItem->getSwitchingElement();

    if (startItemParent == endItemParent)
    {
        myEndItem = (startItem->isInputPort()? endItem : startItem);
        myStartItem = (startItem->isInputPort()? startItem : endItem);
    }
    else
    {
        myStartItem = (startItem->isInputPort()? endItem : startItem);
        myEndItem = (startItem->isInputPort()? startItem : endItem);
    }

    myStartItem->setOutputConnection(this);
    myEndItem->setInputConnection(this);
}

Connection::~Connection()
{
    qDebug()<<"delete Connection1 " ;//<< myStartItem->getFields();
    qDebug()<<"delete Connection2 " ;//<< myEndItem->getFields();
    if (myStartItem)
    {
        myStartItem->removeOutputConnection(this); // Удаляем соединение из списка порта-источника
        myStartItem = nullptr;
    }

    if (myEndItem)
    {
        myEndItem->removeInputConnection(this); // Удаляем соединение из списка порта-получателя
        myEndItem = nullptr;
    }
    emit isDeleted(this);
    emit deleteQConnection();
    qDebug()<<"delete Connection end" ;
}

void Connection::changeEndItem(Port* currentPort)
{
    qDebug()<<"myEndItem "<<myEndItem->getFields();

    myEndItem->removeInputConnection(this);
    myEndItem = currentPort;
    qDebug()<<"currentPort "<<myEndItem->getFields();

    //qDebug()<<"currentPort setInputConnection "<<currentPort->getInputConnections();
    currentPort->setInputConnection(this);
   // qDebug()<<"currentPort setInputConnection "<<newOutputPort->getNumInStage();

    emit endItemChanged(myEndItem);
}
Port* Connection::getEndItem() const {
    qDebug() << "conn->getEndItem() " << myStartItem->getFields();
    qDebug() << "conn->getEndItem() " << myEndItem->getFields();

    return myEndItem; }
