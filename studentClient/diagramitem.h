#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

//#include "diagramscene.h"

#include <QGraphicsItem>
#include "schememode.h"

class DiagramScene;

class DiagramItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    DiagramItem(DiagramScene* dscene, QGraphicsItem* parent = nullptr);
    virtual ~DiagramItem() = default;
    virtual QMap<QString, QString> getFields() const = 0;
    virtual void setFields(QMap<QString, QString> properties) = 0; // Чисто виртуальная функция

    virtual QStringList getHeaderOrder() const = 0;


    //virtual QRectF boundingRect() const override;
    //virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

public slots:
    void updateElementProperties(QMap<QString, QString> properties, DiagramItem* currentObject);
    void setMovable(SchemeMode _mode);

signals:
    void selectItem(DiagramItem* diagramItem);

protected:
    bool movable = false;
    DiagramScene* dscene;  // хранение сцен

};

#endif // DIAGRAMITEM_H
