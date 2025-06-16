#include "qconnection.h"
#include "port.h"
#include "ioport.h"
#include "qswitchingelement.h"
#include "qioport.h"


#include <QPen>
#include <QPainter>
#include <QtMath>

QConnection::QConnection(QPort *startItem, QPort *endItem, Connection* connection, DiagramScene *scene, QGraphicsItem *parent)
    : QGraphicsItem(parent), connection(connection), dscene(scene)
{
    QSwitchingElement* startItemParent = dynamic_cast<QSwitchingElement*>(startItem->parentItem());
    QSwitchingElement* endItemParent = dynamic_cast<QSwitchingElement*>(endItem->parentItem());

    if (startItemParent == endItemParent)
    {
        myEndItem = (startItem->getPort()->isInputPort()? endItem : startItem);
        myStartItem = (startItem->getPort()->isInputPort()? startItem : endItem);
    }
    else
    {
        myStartItem = (startItem->getPort()->isInputPort()? endItem : startItem);
        myEndItem = (startItem->getPort()->isInputPort()? startItem : endItem);
    }

    setZValue(-100);
    //updatePosition();

    connect(scene, DiagramScene::changeSchemeMode, this, QConnection::changedSchemeMode);
    connect(connection, Connection::endItemChanged, this, QConnection::endItemChanged);
    connect(connection, Connection::deleteQConnection, this, QConnection::deleteQConnection);

}

void QConnection::deleteQConnection()
{
    qDebug()<<"delete QConnection1 ";// << myStartItem->getFields();
    qDebug()<<"delete QConnection2 ";// << myEndItem->getFields();
    delete this;
    connection = nullptr;
    qDebug()<<"delete QConnection end" ;
}

void QConnection::updateMyItems()
{
    myStartItem = getQStartItemForStartItem();
    myEndItem = getQEndItemForEndItem();
}

QPort* QConnection::getQStartItemForStartItem()
{
    QPort* nowQPort = dynamic_cast<DiagramScene*>(scene())->getQPorttoPort(connection->getStartItem());
    if (!nowQPort)
    {
        QIOPort* nowQIOPort = dynamic_cast<DiagramScene*>(scene())->getInQIOPorttoPort(connection->getStartItem());
        return nowQIOPort->getQPort();
    }
    else
    {
        return nowQPort;
    }
}

QPort* QConnection::getQEndItemForEndItem()
{
    QPort* nowQPort = dynamic_cast<DiagramScene*>(scene())->getQPorttoPort(connection->getEndItem());
    if (!nowQPort)
    {

        QIOPort* nowQIOPort = dynamic_cast<DiagramScene*>(scene())->getOutQIOPorttoPort(connection->getEndItem());
        return nowQIOPort->getQPort();
    }
    else
    {
        return nowQPort;
    }
}

/*
QConnection::QConnection(QPort *startItem, QPort *endItem, QGraphicsItem *parent)
    : QGraphicsItem(parent)//, Connection(startItem, endItem)
{
    QSwitchingElement* startItemParent = dynamic_cast<QSwitchingElement*>(startItem->parentItem());
    QSwitchingElement* endItemParent = dynamic_cast<QSwitchingElement*>(endItem->parentItem());

    if (startItemParent == endItemParent)
    {
        myEndItem = (startItem->getPort()->isInputPort()? endItem : startItem);
        myStartItem = (startItem->getPort()->isInputPort()? startItem : endItem);
    }
    else
    {
        myStartItem = (startItem->getPort()->isInputPort()? endItem : startItem);
        myEndItem = (startItem->getPort()->isInputPort()? startItem : endItem);
    }

    connection = new Connection(myStartItem->getPort(), myEndItem->getPort());

    setZValue(-100);
    //updatePosition();
}*/

/*
QRectF QConnection::boundingRect() const {
    if (!myStartItem || !myEndItem)
        return QRectF();

    qreal extra = 10;
    return QRectF(myStartItem->scenePos(), myEndItem->scenePos()).normalized().adjusted(-extra, -extra, extra, extra);
}*/
QRectF QConnection::boundingRect() const
{
    /*qreal penWidth = pen().width();
    qreal extra = (penWidth + 20) / 2.0;

    return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(), penWidth + 20))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);*/
    qreal extra = 10;
    return QRectF(myStartItem->scenePos(), myEndItem->scenePos()).normalized().adjusted(-extra, -extra, extra, extra);
    /* qreal penWidth = 2;
        qreal extra = penWidth / 2.0;

        // Определение прямоугольника, охватывающего линию
        qreal left = qMin(myStartItem->scenePos().x(), myEndItem->scenePos().x());
        qreal top = qMin(myStartItem->scenePos().y(), myEndItem->scenePos().y());
        qreal width = qAbs(myEndItem->scenePos().x() - myStartItem->scenePos().x());
        qreal height = qAbs(myEndItem->scenePos().y() - myStartItem->scenePos().y());

        return QRectF(left - extra, top - extra, width + penWidth, height + penWidth);
*/
}

void QConnection::updatePosition()
{
    update();
}

void QConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(myColor); // Создание пера с указанным цветом
    painter->setPen(pen); // Установка пера для рисования линии

    int offsetXStart = 0;
    int offsetYStart = 0;
    int offsetXEnd = 0;
    int offsetYEnd = 0;

    // Получаем координаты центров начального и конечного портов на сцене
    QPointF startPortCenter;
    QPointF endPortCenter;

    if (myStartItem->parentItem() != myEndItem->parentItem())
    {
        if (!(myStartItem->getPort()->isInputPort()))
        {
            startPortCenter = myStartItem->scenePos() + myStartItem->boundingRect().center() + QPointF(offsetXStart, offsetYStart);
            endPortCenter = myEndItem->scenePos() + myEndItem->boundingRect().center() + QPointF(offsetXEnd, offsetYEnd);
        }
        else
        {
            endPortCenter = myStartItem->scenePos() + myStartItem->boundingRect().center() + QPointF(offsetXStart, offsetYStart);
            startPortCenter = myEndItem->scenePos() + myEndItem->boundingRect().center() + QPointF(offsetXEnd, offsetYEnd);
        }

        painter->drawLine(startPortCenter, startPortCenter + QPointF(20,0));
        painter->drawLine(startPortCenter + QPointF(20,0), endPortCenter + QPointF(-20,0));
        painter->drawLine(endPortCenter + QPointF(-20,0),endPortCenter);
    }
    else
    {
        startPortCenter = myStartItem->scenePos() + myStartItem->boundingRect().center() + QPointF(offsetXStart, offsetYStart);
        endPortCenter = myEndItem->scenePos() + myEndItem->boundingRect().center() + QPointF(offsetXEnd, offsetYEnd);

        painter->drawLine(startPortCenter, endPortCenter);
    }
    updatePosition();
}

void QConnection::changedSchemeMode(SchemeMode _mode)
{
    setColor(Qt::black);
}

void QConnection::endItemChanged(Port* newPort)
{
    myEndItem = dscene->getQPorttoPort(newPort);
}



/*
QPainterPath QConnection::shape() const {
    QPainterPath path;
    QList<QPointF> points = getLinePoints();
    if (points.isEmpty())
        return path;

    path.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
        path.lineTo(points[i]);

    QPainterPathStroker stroker;
    stroker.setWidth(4);
    return stroker.createStroke(path);
}

bool QConnection::contains(const QPointF &point) const {
    return shape().contains(point);
}

void QConnection::updatePosition() {
    if (!myStartItem || !myEndItem)
        return;

    prepareGeometryChange();
    update();
}

QList<QPointF> QConnection::getLinePoint() {

    QList<QPointF> linePoints;

    int offsetXStart = 0;
    int offsetYStart = 0;
    int offsetXEnd = 0;
    int offsetYEnd = 0;

    IOPort *ioPort = dynamic_cast<IOPort*>(myStartItem);
    if (ioPort){
        if (ioPort->isInputPort()){
            offsetXStart = -15;
        }
        else{
            offsetXStart = 15;
        }
    }

    ioPort = dynamic_cast<IOPort*>(myEndItem);
    if (ioPort){
        if (ioPort->isInputPort()){
            offsetXEnd = -15;
        }
        else{
            offsetXEnd = 15;
        }
    }

    // Получаем координаты центров начального и конечного портов на сцене
    QPointF startPortCenter;
    QPointF endPortCenter;

    if (myStartItem->parentItem() != myEndItem->parentItem())
    {

        if (!(myStartItem->isInputPort())){
            startPortCenter = myStartItem->scenePos() + myStartItem->boundingRect().center() + QPointF(offsetXStart, offsetYStart);
            endPortCenter = myEndItem->scenePos() + myEndItem->boundingRect().center() + QPointF(offsetXEnd, offsetYEnd);
        }
        else{
            endPortCenter = myStartItem->scenePos() + myStartItem->boundingRect().center() + QPointF(offsetXStart, offsetYStart);
            startPortCenter = myEndItem->scenePos() + myEndItem->boundingRect().center() + QPointF(offsetXEnd, offsetYEnd);
        }
        linePoints.append(startPortCenter);
        linePoints.append(startPortCenter + QPointF(20,0));
        linePoints.append(endPortCenter + QPointF(-20,0));
        linePoints.append(endPortCenter);
    }
    else {
        startPortCenter = myStartItem->scenePos() + myStartItem->boundingRect().center() + QPointF(offsetXStart, offsetYStart);
        endPortCenter = myEndItem->scenePos() + myEndItem->boundingRect().center() + QPointF(offsetXEnd, offsetYEnd);

        linePoints.append(startPortCenter);
        linePoints.append(endPortCenter);
    }
    return linePoints;
}

*/

