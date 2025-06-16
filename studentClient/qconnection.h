#ifndef QCONNECTION_H
#define QCONNECTION_H

#include <QPainter>

//#include "connection.h"
//#include "qport.h"
#include "diagramscene.h"
#include "schememode.h"

class QPort;
class Connection;

class QConnection : public QObject, public QGraphicsItem
{

    Q_OBJECT

public:
    enum { Type = UserType + 4 };

    QConnection(QPort *startItem, QPort *endItem, Connection* connection, DiagramScene *scene, QGraphicsItem *parent = nullptr);
    Connection* getConnection() { return connection; }

    QRectF boundingRect() const override;
    void updatePosition();

    QColor getColor() { return myColor; }
    void setColor(QColor _myColor) { myColor = _myColor; }

    void updateMyItems();
    QPort* getQStartItemForStartItem();
    QPort* getQEndItemForEndItem();

    QPort* getQStartItem() { return myStartItem; }
    QPort* getQEndItem()  { return myEndItem; }


    static QPixmap image() {
        QPixmap pixmap(110, 110);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.translate(57, 55);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::black, 2));        // Рисуем линию

        int circleRadius = 2;        // Задаем параметры линии и кругов

        QPointF startPoint(-40, 40);        // Вычисляем координаты точек начала и конца линии
        QPointF endPoint(40, -40);
        painter.drawLine(startPoint, endPoint);

        //painter.setBrush(Qt::black);        // Рисуем круги на концах линии
        painter.drawEllipse(startPoint, circleRadius * 2, circleRadius * 2);
        painter.drawEllipse(endPoint, circleRadius * 2, circleRadius * 2);
        painter.end();        // Завершаем рисование

        return pixmap;
    }

    /*int type() const override { return Type; }
    QPainterPath shape() const override;
    bool contains(const QPointF &point) const override;

    void setColor(const QColor &color) { myColor = color; }
    QColor getColor() const { return myColor; }
*/

public slots:
    void changedSchemeMode(SchemeMode _mode);
    void endItemChanged(Port* newPort);
    void deleteQConnection();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QColor myColor = Qt::black;

    QPort *myStartItem;
    QPort *myEndItem;

    Connection* connection;
    DiagramScene* dscene;
};

#endif // QCONNECTION_H
