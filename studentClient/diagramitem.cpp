#include "diagramitem.h"
#include "propertiestable.h"
#include <QTimer>

#include "mainwindow.h"

DiagramItem::DiagramItem(DiagramScene* dscene, QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    connect(this, &DiagramItem::selectItem, MainWindow::_propertiesTable, &PropertiesTable::setCurrentObject);
    //connect(MainWindow::_propertiesTable, &PropertiesTable::propertiesChanged, this, &DiagramItem::updateElementProperties);

    connect(dscene, DiagramScene::changeSchemeMode, this, DiagramItem::setMovable);

}

void DiagramItem::updateElementProperties(QMap<QString, QString> properties, DiagramItem* currentObject)
{
    if (currentObject == this)
    {
        currentObject->setFields(properties);
    }

    currentObject->update();
}

void DiagramItem::setMovable(SchemeMode _mode)
{
    bool _movable = false;
    if (_mode == SchemeMode::Edit)
    {
        _movable = true;
    }
    else if (_mode == SchemeMode::Run)
    {
        _movable = false;
    }
    else if (_mode == SchemeMode::LockSimulation)
    {
        _movable = false;
    }
    else if (_mode == SchemeMode::buildPath)
    {
        _movable = false;
    }

    if (movable)
    {
        setFlag(QGraphicsItem::ItemIsMovable, _movable);
    }
}
