//#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsRectItem>
#include <iostream>
#include <QGraphicsTextItem>
#include <QLabel>
#include <QGraphicsProxyWidget>
#include <QList>
#include <QRandomGenerator>

#include "qswitchingelement.h"
#include "qconnection.h"
#include "closa.h"
#include "prioritysettingsdialog.h"
#include "scheme.h"


QStringList QSwitchingElement::headerOrder = {"Количество входов", "Количество выходов", "Ступень", "Номер в ступени"};


QSwitchingElement::QSwitchingElement(QMenu *contextMenu, SwitchingElement* switchingElement, QGraphicsItem *parent, DiagramScene *scene)
    : DiagramItem(scene, parent), diagramScene(scene), switchingElement(switchingElement)
{
    setAcceptHoverEvents(true);
    setZValue(98);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    QList<Port*> inputPorts = switchingElement->getInputPorts();
    QList<Port*> outputPorts = switchingElement->getOutputPorts();

    setInputQPorts(inputPorts);
    setOutputQPorts(outputPorts);

    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

    setInConnections();

    movable = true;

    connect(scene, DiagramScene::changeSchemeMode, this, QSwitchingElement::changedSchemeMode);

    /*  createInConnections(scene);

    label = new QGraphicsTextItem(this);
    label->setDefaultTextColor(Qt::black);
    label->setFont(QFont("Arial", 12));
    label->setPlainText("Switching Element");*/
}

QSwitchingElement::~QSwitchingElement()
{
    qDebug()<<"delete QSwitchingElement " ;//<< getFields();

    qDeleteAll(inputQPorts);
    qDeleteAll(outputQPorts);

    inputQPorts.clear();
    outputQPorts.clear();

    delete switchingElement; // Удаляем сам коммутатор
    switchingElement = nullptr;
    qDebug()<<"delete QSwitchingElement end" ;

}

/*QSwitchingElement::QSwitchingElement(QMenu *contextMenu, int inp, int outp, QGraphicsItem *parent, DiagramScene *scene)
    : DiagramItem(parent), diagramScene(scene)
{
    setAcceptHoverEvents(true);
    setZValue(98);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    switchingElement = new SwitchingElement(inp, outp);

    QList<Port*> inputPorts = switchingElement->getInputPorts();
    QList<Port*> outputPorts = switchingElement->getOutputPorts();

    setInputQPorts(inputPorts);
    setOutputQPorts(outputPorts);

    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
*/
  /*  createInConnections(scene);

    label = new QGraphicsTextItem(this);
    label->setDefaultTextColor(Qt::black);
    label->setFont(QFont("Arial", 12));
    label->setPlainText("Switching Element");*/
//}


void QSwitchingElement::setInConnections()
{
    QList<QPort*> availableOutputs = getOutputQPorts();
    //QList<QPort*> availableInputs = getInputQPorts();
    QList<QPort*> availableInputs;


    for (QPort* inputPort : getInputQPorts())
    {
        if (inputPort->getPort() && inputPort->getPort()->getOutputConnections().isEmpty())
        {
            availableInputs.append(inputPort);
        }
    }
    if (availableOutputs.count() == 0 || availableInputs.count() == 0) return;

    while (availableOutputs.size() < availableInputs.size())
    {
        availableOutputs.append(availableOutputs[QRandomGenerator::global()->bounded(availableOutputs.size())]);
    }

    std::shuffle(availableOutputs.begin(), availableOutputs.end(), *QRandomGenerator::global());
    std::shuffle(availableInputs.begin(), availableInputs.end(), *QRandomGenerator::global());

    int index = availableInputs.size();
    qDebug() << "index " << index;
    qDebug() << "availableInputs " << availableInputs.count();
    qDebug() << "availableOutputs " << availableOutputs.count();

    for (int i = 0; i < index; i++)
    {
        qDebug() << "setInConnections " << index;

        QPort* currInputPort = availableInputs[i];
        QPort* currOutputPort = availableOutputs[i];

        QConnection* qConnection = diagramScene->addConnection(currInputPort, currOutputPort, true);

        /*Connection* connection = new Connection(currInputPort->getPort(), currOutputPort->getPort(), true);
        QConnection *connItem = new QConnection(currInputPort, currOutputPort, connection, diagramScene);
        diagramScene->addItem(connItem);
        diagramScene->addConnectionToQConnection(connection, connItem);*/

        switchingElement->addConnection(qConnection->getConnection());
    }
}

void QSwitchingElement::changedSchemeMode(SchemeMode _mode)
{
    switchingElement->clearControlPacket();
}


void QSwitchingElement::setInputQPorts(QList<Port*> inputPorts)
{
    qDebug() << "setInputQPorts";

    qreal width = 10; // Ширина места соединения
    qreal height = 10; // Высота места соединения
    qreal startY;

    int qportsCount = inputQPorts.count();  // сейчас
    int numPorts = inputPorts.count();      // нужно

    if (qportsCount != numPorts)
    {
        updateRect(fmax(numPorts, outputQPorts.count()));

        if (numPorts > qportsCount)
        {

            for (int i = 0; i < numPorts - qportsCount; i++)
            {
                QPort* newQPort = new QPort(0, 0, width, height, inputPorts[inputQPorts.count()], diagramScene, this);
                inputQPorts.append(newQPort);
            }

            startY = (rect.height() - (numPorts - 1) * 20) / 2;

            QList<QPort *> _inputQPorts = getInputQPorts();
            foreach (QPort* _qport, _inputQPorts)
            {
                _qport->setPos((-rect.width() - width)/2, (-rect.height() - height)/2 + startY);
                startY += 20;
            }
        } else
        {
            for (int i = inputQPorts.count() - 1; i >= 0; --i)
            {
                QPort* qport = inputQPorts[i];
                if (qport->getPort() == nullptr)
                {
                    qDebug() << "delete 1 qport";

                    delete qport;
                    inputQPorts.removeAt(i);

                    qDebug() << "delete 1 qport end";
                }

                startY = (rect.height() - (numPorts - 1) * 20) / 2;

                QList<QPort *> _inputQPorts = getInputQPorts();
                foreach (QPort* _qport, _inputQPorts)
                {
                    _qport->setPos((-rect.width() - width)/2, (-rect.height() - height)/2 + startY);
                    startY += 20;
                }
            }
        }
        setInConnections();
    }
}

void QSwitchingElement::setOutputQPorts(QList<Port*> outputPorts)
//void QSwitchingElement::setOutputQPorts(int outp)
{
    //QList<Port*> outputPorts = switchingElement->getInputPorts();

    qreal width = 10; // Ширина места соединения
    qreal height = 10; // Высота места соединения

    int qportsCount = outputQPorts.count();  // сейчас
    int numPorts = outputPorts.count();      // нужно

    if (numPorts != qportsCount)
    {
        updateRect(fmax(inputQPorts.count(), numPorts));

        if (numPorts > qportsCount)
        {
            for (int i = 0; i < numPorts - qportsCount; i++)
            {
                outputQPorts.append(new QPort(0, 0, width, height, outputPorts[outputQPorts.count()], diagramScene, this));
            }

            qreal startY = (rect.height() - (numPorts - 1) * 20) / 2;
            QList<QPort *> _outputQPorts = getOutputQPorts();

            foreach (QPort* _qport, _outputQPorts)
            {
                _qport->setPos((rect.width() - width)/2, (-rect.height() - height)/2 + startY );
                startY += 20;
            }
        } else
        {
            for (int i = outputQPorts.count() - 1; i >= 0; --i)
            {
                QPort* qport = outputQPorts[i];
                if (qport->getPort() == nullptr)
                {
                    qDebug() << "delete 1 qport out";

                    delete qport;
                    outputQPorts.removeAt(i);

                    qDebug() << "delete 1 qport out end ";
                }

                qreal startY = (rect.height() - (numPorts - 1) * 20) / 2;

                QList<QPort *> _outputQPorts = getOutputQPorts();
                foreach (QPort* _qport, _outputQPorts)
                {
                    _qport->setPos((rect.width() - width)/2, (-rect.height() - height)/2 + startY );
                    startY += 20;
                }
            }
        }
        setInConnections();
    }

}

void QSwitchingElement::updateRect(int numPorts)
{
    int polygonWidth = 60;
    int polygonHeight = (numPorts + 1) * 20;

    QRectF _rect(-polygonWidth / 2, -polygonHeight / 2, polygonWidth, polygonHeight);
    this->rect = _rect;
    prepareGeometryChange(); // Уведомляем систему о том, что геометрия изменится

    //setRect(rect);
}


QRectF QSwitchingElement::boundingRect() const
{
    return rect;
}

void QSwitchingElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRect(rect);


    QList<QString> controlPacketText = switchingElement->getControlPacket();
    if (!controlPacketText.isEmpty())
    {
        QFontMetricsF fontMetrics(painter->font());

        for (int channelNum = 0; channelNum < controlPacketText.size(); channelNum++)
        {
            const QString& packet = controlPacketText[channelNum]; // Текущий управляющий пакет
            qreal textWidth = fontMetrics.horizontalAdvance(packet);

            QPointF textPosition = QPointF(rect.center().x() - textWidth / 2,  // Центрируем текст
                                           rect.top() - fontMetrics.height() * (channelNum + 1) - 5); // Над элементом

            // Отображаем каждый символ пакета
            for (int i = 0; i < packet.length(); ++i)
            {
                qreal charX = textPosition.x() + fontMetrics.horizontalAdvance(packet.left(i));
                qreal charY = textPosition.y();

                if (i == getStage().toInt())
                {  // Выделяем символ текущей ступени
                    painter->save();
                    QFont boldFont = painter->font();
                    boldFont.setBold(true);
                    painter->setFont(boldFont);
                    painter->setPen(Qt::red);
                    painter->drawText(QPointF(charX, charY), packet.mid(i, 1));
                    painter->restore();
                } else
                {
                    painter->setPen(Qt::black);
                    painter->drawText(QPointF(charX, charY), packet.mid(i, 1));
                }
            }
        }
    }


    // Рисуем значок настроек
    if (dynamic_cast<Closa*>(dynamic_cast<DiagramScene*>(scene())->getScheme()->getTopology()) && getStage().toInt() == 0)
    {
        QRectF settingsIcon(rect.topRight() - QPointF(20, 0), QSizeF(20, 20));
        painter->setBrush(Qt::blue);
        painter->drawEllipse(settingsIcon);
    }


    scene()->update();
}

void QSwitchingElement::openPrioritySettingsDialog()
{
    //if (dynamic_cast<Closa*>();

    Topology* topology = dynamic_cast<DiagramScene*>(scene())->getScheme()->getTopology();
    Closa* topologyClosa = dynamic_cast<Closa*>(topology);

    if (!topologyClosa) return;  // Проверяем, есть ли ссылка на топологию

    qDebug() << "switchingElement->getStage().toInt() "<< switchingElement->getStage().toInt();
    qDebug() << "switchingElement->getOutputCount() "<< switchingElement->getOutputCount();
    PrioritySettingsDialog dialog(topologyClosa, switchingElement->getStage().toInt(), switchingElement->getOutputCount());
    if (dialog.exec() == QDialog::Accepted)
    {
        topologyClosa->setStagePriorities(switchingElement->getStage().toInt(), dialog.getUpdatedPriorities());
        update();  // Перерисовать элемент
    }
}

void QSwitchingElement::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QRectF settingsIcon(rect.topRight() - QPointF(20, 0), QSizeF(20, 20));

    if (settingsIcon.contains(event->pos()) && switchingElement->getOutputCount() > 2)
    {
        openPrioritySettingsDialog();
    } else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

/*
void QSwitchingElement::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->popup(event->screenPos());
}

QVariant QSwitchingElement::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == QGraphicsItem::ItemPositionChange) {
        for (Arrow *arrow : qAsConst(arrows)) {
            arrow->updatePosition();
        }
    }
    return value;
}
*/

void QSwitchingElement::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    foreach (QPort *qport, inputQPorts)
    {
        qport->showPort();
    }

    foreach (QPort *qport, outputQPorts)
    {
        qport->showPort();
    }
}

void QSwitchingElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    foreach (QPort *qport, inputQPorts)
    {
        qport->hidePort();
    }

    foreach (QPort *qport, outputQPorts)
    {
        qport->hidePort();
    }
}

/*void QSwitchingElement::createInConnections(DiagramScene *scene) {
    // Logic to create connections
}*/

void QSwitchingElement::setFields(QMap<QString, QString> properties)
{
    switchingElement->setFields(properties);

    QList<Port*> inputPorts = switchingElement->getInputPorts();
    QList<Port*> outputPorts = switchingElement->getOutputPorts();

    setInputQPorts(inputPorts);
    setOutputQPorts(outputPorts);
}
