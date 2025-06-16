#ifndef PROPERTIESTABLE_H
#define PROPERTIESTABLE_H

#include <QDockWidget>
#include <QTableWidget>
#include <QGraphicsItem>
#include <QPushButton>

class DiagramItem;

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class PropertiesTable : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesTable(QWidget *parent = nullptr);

    DiagramItem *getCurrentObject() const;
    void updatePropertiesTable();

    void setModified(bool modified);

public slots:
    void setCurrentObject(DiagramItem *newCurrentObject);

signals:
    //void propertiesChanged(QMap<QString, QString> properties, DiagramItem *CurrentObject);
    void propertiesChanged(QString operation, QGraphicsItem* item, QMap<QString, QString> properties);

private slots:
    void handleItemChanged(QTableWidgetItem *item);
    void applyChanges();
    void discardChanges();

private:

    DiagramItem *currentObject;
    QTableWidget *tableWidget; // Вложенная таблица


    bool isModified = false;

    QPushButton* applyButton;
    QPushButton* cancelButton;

};

#endif // PROPERTIESTABLE_H
