#include "qport.h"
#include "diagramscene.h" // Полное определение DiagramScene

QStringList QPort::headerOrder = {"Номер в ступени", "Номер в элементе"};


QPort::QPort(qreal x, qreal y, qreal width, qreal height, Port* _port, DiagramScene *scene, QGraphicsItem* parent)
    : /*Port(isInputPort, ioNum), */DiagramItem(scene, parent), rect(x, y, width, height), port(_port)
{
    borderColor = Qt::transparent;
    fillColor = Qt::white;
    fillColor.setAlpha(0);
    setAcceptHoverEvents(true);
    connect(this, &QPort::clicked, scene, &DiagramScene::addPortForConnectoin);
    setZValue(100);

    movable = false;
}

QPort::~QPort()
{
    qDebug()<<"delete qPort " ;//<< getFields();
    //port = nullptr; // Просто обнуляем, так как сам порт удалится в `SwitchingElement`
    if (port != nullptr) delete port;
    port = nullptr;
    qDebug()<<"delete qPort end" ;
}

QRectF QPort::boundingRect() const
{
    return rect;
}

void QPort::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->save();
    painter->setPen(borderColor);
    painter->setBrush(fillColor);
    painter->drawRect(rect);
    painter->restore();
}

void QPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit clicked(this);

    port->switchToNextOutput();

    QGraphicsItem::mousePressEvent(event);
}

void QPort::setBorderColor(const QColor& color)
{
    borderColor = color;
    update();
}

void QPort::setFillColor(const QColor& color)
{
    fillColor = color;
    update();
}

void QPort::setFillColorAlpha(int alpha)
{
    fillColor.setAlpha(alpha);
    update();
}

void QPort::showPort()
{
    borderColor = Qt::red;
    fillColor.setAlpha(255);
    update();
}

void QPort::hidePort()
{
    borderColor = Qt::transparent;
    fillColor.setAlpha(0);
    update();
}
