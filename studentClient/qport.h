#ifndef QPORT_H
#define QPORT_H

#include <QPainter>
#include <QColor>
#include <QPointer>

#include "port.h"
//#include "diagramscene.h" // Полное определение DiagramScene
#include "diagramitem.h"
#include "schememode.h"

class DiagramScene;

class QPort : public DiagramItem
{

    Q_OBJECT

public:
    QPort(qreal x, qreal y, qreal width, qreal height, Port* _port, DiagramScene *scene,  QGraphicsItem* parent = nullptr);
    ~QPort();
    Port *getPort() const { return port; }


    static QStringList headerOrder;
    QStringList getHeaderOrder() const override { return headerOrder; }


    // Графические функции
    QRectF boundingRect() const override;    // Реализация интерфейса QGraphicsItem
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void setBorderColor(const QColor& color);
    void setFillColor(const QColor& color);
    void setFillColorAlpha(int alpha);

    QColor getBorderColor() const { return borderColor; }
    QColor getFillColor() const { return fillColor; }
    int getFillColorAlpha() const { return fillColor.alpha(); }

    void showPort();
    void hidePort();

    QMap<QString, QString> getFields() const
    {
        QMap<QString, QString> portFields =  port->getFields();

        /*if (port->getSwitchingElement() == nullptr)
        {
            QString value = portFields.take("Номер в ступени");
            portFields["Номер"] = value;

            portFields.remove("Номер в  элементе");
        }*/

        return portFields;
    }

    void setFields(QMap<QString, QString> properties)
    {
        port->setFields(properties);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void clicked(QPort* port);

private:
    QPointer <Port> port;

    QRectF rect;
    QColor borderColor;
    QColor fillColor;

};

#endif // QPORT_H
