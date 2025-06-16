#ifndef QIOPORT_H
#define QIOPORT_H

#include <QGraphicsItem>
#include <QPainter>

#include "ioport.h"
#include "qport.h"
#include "diagramitem.h"

class QIOPort : public DiagramItem
{

    Q_OBJECT

public:
    //QIOPort(qreal x, qreal y, qreal width, qreal height, bool isInputPort, DiagramScene *scene, QGraphicsItem* parent = nullptr);
    QIOPort(qreal x, qreal y, qreal width, qreal height, Port* newPort, DiagramScene *scene, QGraphicsItem* parent = nullptr);
    ~QIOPort();


    static QPixmap Image(int ioNum, bool inputPort) {

        QPixmap pixmap(100, 100); // Увеличим ширину, чтобы было место для текста
        pixmap.fill(Qt::transparent); // Заполняем фон прозрачным цветом

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        if (inputPort){
            // Рисуем прямоугольник (порт)
            QRectF portRect(5, 30, 90, 40); // Перемещаем прямоугольник вправо
            painter.setPen(QPen(Qt::black, 2));
            painter.setBrush(Qt::transparent); // Цвет заливки
            painter.drawRect(portRect);

            // Рисуем прямоугольник (порт)
            portRect = QRectF(65, 40, 20, 20); // Перемещаем прямоугольник вправо
            painter.drawRect(portRect);

            // Настраиваем шрифт для отображения номера порта
            QFont font = painter.font();
            font.setPointSize(20); // Устанавливаем размер шрифта
            painter.setFont(font);
            // Рисуем надпись с номером порта слева
            QRectF textRect(10, 30, 60, 40); // Область для текста
            painter.setPen(Qt::black); // Цвет текста
            painter.drawText(textRect, Qt::AlignLeft, "XXX"); // Отрисовываем номер порта слева
        }else
        {
            // Рисуем прямоугольник
            QRectF portRect(5, 30, 90, 40);
            painter.setPen(QPen(Qt::black, 2));
            painter.setBrush(Qt::transparent); // Цвет заливки
            painter.drawRect(portRect);

            // Рисуем порт
            portRect = QRectF(15, 40, 20, 20);
            painter.drawRect(portRect);

            // Настраиваем шрифт для отображения номера порта
            QFont font = painter.font();
            font.setPointSize(20); // Устанавливаем размер шрифта
            painter.setFont(font);
            // Рисуем надпись с номером порта слева
            QRectF textRect(30, 30, 60, 40); // Область для текста
            painter.setPen(Qt::black); // Цвет текста
            painter.drawText(textRect, Qt::AlignRight, "XXX"); // Отрисовываем номер порта слева
        }

        return pixmap;
    }

    static QStringList headerOrder;
    QStringList getHeaderOrder() const override { return headerOrder; }


    void setBorderColor(const QColor &color);
    void setFillColor(const QColor &color);
    void setFillColorAlpha(int alpha);

    QColor getBorderColor() const { return borderColor; }
    QColor getFillColor() const { return fillColor; }
    int getFillColorAlpha() const { return fillColor.alpha(); }

    QPort* getQPort() { qDebug()<<"qPort" << !qPort; return qPort; }

    QMap<QString, QString> getFields() const
    {

        QMap<QString, QString> portFields =  qPort->getFields();

        QString value = portFields.take("Номер в ступени");
        portFields["Номер"] = value;

        portFields.remove("Номер в элементе");

        return portFields;
    }

    void setFields(QMap<QString, QString> properties)
    {
        QString value = properties.take("Номер");
        properties["Номер в ступени"] = value;

        qPort->getPort()->setFields(properties);
    }

protected:
//    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
//    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;   
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event);


private:
    void setQPort(Port* port, DiagramScene *scene);

    //IOPort* ioPort;
    QPort* qPort;

    QColor borderColor; // Объявление переменной borderColor
    QColor fillColor; // цвет заливки
    QRectF rect;

};

#endif // QIOPORT_H
