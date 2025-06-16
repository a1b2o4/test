#ifndef QSWITCHINGELEMENT_H
#define QSWITCHINGELEMENT_H

#include "switchingelement.h"
#include "qport.h"
//#include "diagramscene.h"
#include "schememode.h"

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QMenu>
#include <QLabel>
#include <QGraphicsPixmapItem>
#include <QList>
#include <QPainter>
#include <QGraphicsScene>

class DiagramScene;
class QPort;

class QSwitchingElement: public DiagramItem
{

    Q_OBJECT

public:
    //QSwitchingElement(QMenu *contextMenu, int inp, int outp, QGraphicsItem *parent, DiagramScene *scene = nullptr);
    QSwitchingElement(QMenu *contextMenu, SwitchingElement *switchingElement, QGraphicsItem *parent, DiagramScene *scene = nullptr);
    ~QSwitchingElement();

    int type() const override { return UserType + 15; }

    void setInputQPorts(QList<Port*> inputPorts);
    void setOutputQPorts(QList<Port*> outputPorts);
    void setInConnections();
    //void setInputQPorts(int inp);
    //void setOutputQPorts(int outp);

    QList<QPort *> getInputQPorts() const { return inputQPorts; }
    QList<QPort *> getOutputQPorts() const { return outputQPorts; }

    void updateRect(int numPorts);

    static QStringList headerOrder;
    QStringList getHeaderOrder() const override { return headerOrder; }

    static QPixmap image(int numInputPorts, int numOutputPorts) {
        QColor buttonColor(230, 230, 230);

        // Размеры портов и расстояние между ними
        const int portSize = 16;
        const int inputPortSpacing = 80 / (numInputPorts + 1);
        const int outputPortSpacing = 80 / (numOutputPorts + 1);

        // Создаем pixmap и инициализируем его прозрачным цветом
        QPixmap pixmap(110, 110);
        pixmap.fill(Qt::transparent);

        // Создаем painter
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::black, 2));
        painter.translate(55, 55); // Центрируем элемент

        // Рисуем основной прямоугольник элемента
        QPolygonF polygon;
        polygon << QPointF(-40, -50) << QPointF(40, -50)
                << QPointF(40, 50) << QPointF(-40, 50)
                << QPointF(-40, -50);
        painter.drawPolyline(polygon);

        // Рисование входных портов цветом кнопки
        //painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(buttonColor);
        int yPos = -50 + outputPortSpacing - 5; // Начальная позиция портов
        for (int i = 0; i < outputPortSpacing; ++i) {
            painter.drawRect(-48, yPos, portSize, portSize);
            yPos += portSize + outputPortSpacing;
        }

        // Рисование выходных портов цветом кнопки
        yPos = -50 + inputPortSpacing - 5; // Начальная позиция портов
        for (int i = 0; i < numInputPorts; ++i) {
            painter.drawRect(32, yPos, portSize, portSize);
            yPos += portSize + inputPortSpacing;
        }

        return pixmap;
    }

    int getInputCount() const { return inputQPorts.count(); }
    int getOutputCount() const { return outputQPorts.count(); }

    QString getStage() const { return switchingElement->getStage(); }
    QString getNumInStage() const { return switchingElement->getStage(); }

    SwitchingElement* getSwitchingElement() { return switchingElement; }

    QMap<QString, QString>  getFields() const { return switchingElement->getFields(); }
    void setFields(QMap<QString, QString> properties);

    void openPrioritySettingsDialog();


public slots:
    void changedSchemeMode(SchemeMode _mode);

protected:
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    //QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    QRectF boundingRect() const override;

private:
    QRectF rect;
    QMenu *contextMenu;
    DiagramScene *diagramScene;

    SwitchingElement* switchingElement;
    QList<QPort*> inputQPorts;
    QList<QPort*> outputQPorts;

    //QList<QString> controlPacketText;
    //QList<int> highlightedBit;
    QList<QColor> textColor;

    QLabel *textLabel;
    QGraphicsTextItem *label;

    void updateText();
    //void createInConnections(DiagramScene *scene);

};

#endif //QSWITCHINGELEMENT_H
