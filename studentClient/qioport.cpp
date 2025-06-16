#include "qioport.h"

#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QAction>
#include <QFont>

QStringList QIOPort::headerOrder = {"Номер"};


QIOPort::QIOPort(qreal x, qreal y, qreal width, qreal height, Port* newPort, DiagramScene *scene, QGraphicsItem* parent)
    : DiagramItem(scene, parent), rect(x, y, width, height)
{
    setAcceptHoverEvents(true);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setZValue(100);

    //Port* newPort = new Port(!isInputPort, 0, nullptr);

    setQPort(newPort, scene);

    setZValue(99);

    movable = true;
}

QIOPort::~QIOPort()
{
    qDebug()<<"delete qioPort " ;//<< getFields();
    delete qPort;
    qPort = nullptr;
    qDebug()<<"delete qioPort end" ;
}

void QIOPort::setQPort(Port* port, DiagramScene *scene)
{
    qreal width = 10; // Ширина места соединения
    qreal height = 10; // Высота места соединения

    qPort = new QPort(0, 0, width, height, port, scene, this);

    bool inputPort = !port->isInputPort();
    if (inputPort)
    {
        qPort->setPos(rect.width() / 2 + width, (-rect.height() + height) / 2);
    }
    else
    {
        qPort->setPos(-rect.width() / 2 + width, (-rect.height() + height) / 2);
    }
}

QRectF QIOPort::boundingRect() const
{
    return rect;
}

void QIOPort::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->drawRect(boundingRect());

    QFont font = painter->font();    // Настраиваем шрифт для отображения цифры
    font.setPointSize(10); // Размер шрифта
    painter->setFont(font);
    QRectF textRect;

    //QString binaryPortNum = QString("%1").arg(portNum, 4, 2, QChar('0')); // Форматируем номер порта в двоичную строку с нулями впереди
    bool inputPort = !qPort->getPort()->isInputPort();
    if (inputPort)
    {
        textRect = boundingRect().adjusted(-10, -2, 0, 0); // Прямоугольник для текста
    }
    else
    {
        textRect = boundingRect().adjusted(10, -2, 0, 0); // Прямоугольник для текста
    }

    painter->setPen(Qt::black); // Цвет текста
    painter->drawText(textRect, Qt::AlignCenter, qPort->getPort()->getNumInStage());//getBinaryPortNum()); // Отображаем номер порта
}

void QIOPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    Q_UNUSED(event)

    qPort->showPort();
}

void QIOPort::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    Q_UNUSED(event)

    qPort->hidePort();
}

void QIOPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit qPort->clicked(qPort);

    QGraphicsItem::mousePressEvent(event);
}

/*
void QIOPort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    QMenu menu;
    QAction *action = new QAction("Execute Network", nullptr);
    connect(action, &QAction::triggered, [this]() {
        // Логика обработки
    });
    menu.addAction(action);
    menu.exec(event->screenPos());
}

void QIOPort::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    // Здесь можно обработать нажатие мыши
    QGraphicsItem::mousePressEvent(mouseEvent);
}
*/

