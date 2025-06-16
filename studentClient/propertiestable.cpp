#include "propertiestable.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QItemDelegate>
#include <QLineEdit>
#include <QHeaderView>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "qswitchingelement.h"
#include "qioport.h"
#include "diagramitem.h"

class DigitOnlyDelegate : public QItemDelegate
{
    public:
        DigitOnlyDelegate(PropertiesTable *parentTable, QObject *parent = nullptr)
            : QItemDelegate(parent), table(parentTable) {}

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
            QLineEdit *editor = new QLineEdit(parent);
            if (index.row() == 0 || index.row() == 1) {
                QRegularExpression regExp("[0-9]*");
                QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, editor);
                editor->setMaxLength(5);
                editor->setValidator(validator);
            }

            connect(editor, &QLineEdit::textChanged, editor, [this]() {  // Добавлено "this"
                qDebug() << "textChanged tab " << true;
                if (this->table) {
                    this->table->setModified(true);
                }
            });

            return editor;
        }

    private:
        PropertiesTable *table;
};

PropertiesTable::PropertiesTable(QWidget *parent)
    : QDockWidget(parent), currentObject(nullptr), isModified(false)
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);

    // Кнопки подтверждения и отмены
    applyButton = new QPushButton("✅", this);
    cancelButton = new QPushButton("❌", this);
    applyButton->setStyleSheet("QPushButton:disabled { background-color: lightgray; color: gray; border: 1px solid gray; }"
                          "QPushButton:disabled:hover { background-color: lightgray; color: gray; border: 1px solid gray; }");
    cancelButton->setStyleSheet("QPushButton:disabled { background-color: lightgray; color: gray; border: 1px solid gray; }"
                          "QPushButton:disabled:hover { background-color: lightgray; color: gray; border: 1px solid gray; }");


    connect(applyButton, &QPushButton::clicked, this, &PropertiesTable::applyChanges);
    connect(cancelButton, &QPushButton::clicked, this, &PropertiesTable::discardChanges);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels({"Свойство", "Значение"});
    tableWidget->setItemDelegate(new DigitOnlyDelegate(this, this));
    tableWidget->verticalHeader()->setVisible(false);

    layout->addLayout(buttonLayout);
    layout->addWidget(tableWidget);
    container->setLayout(layout);
    setWidget(container);

    connect(tableWidget, &QTableWidget::itemChanged, this, &PropertiesTable::handleItemChanged);

    setWindowTitle("Properties");
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);

    setModified(false);

}

DiagramItem *PropertiesTable::getCurrentObject() const
{
    return currentObject;
}

void PropertiesTable::setCurrentObject(DiagramItem *newCurrentObject)
{
    currentObject = newCurrentObject;
    updatePropertiesTable();
    show();

    setModified(false);
    qDebug()<<"setcurrent tab";
}

void PropertiesTable::handleItemChanged(QTableWidgetItem *item)
{
    if (!currentObject) return;
}

void PropertiesTable::updatePropertiesTable()
{
    tableWidget->clearContents();

    if (currentObject != nullptr)
    {
        QMap<QString, QString> fields = currentObject->getFields();
        qDebug() << "fields " << fields;
        QStringList Headers = currentObject->getHeaderOrder();
        tableWidget->setRowCount(Headers.count());

        int rowNum = 0;
        foreach (QString header, currentObject->getHeaderOrder())
        {
            tableWidget->setItem(rowNum, 0, new QTableWidgetItem(header));
            tableWidget->setItem(rowNum, 1, new QTableWidgetItem(fields[header]));
            rowNum++;
        }
    }
    setModified(false);
    qDebug()<<"update tab";
}

// Применить изменения
void PropertiesTable::applyChanges()
{
    if (!currentObject) return;

    QMap<QString, QString> properties;
    for (int i = 0; i < tableWidget->rowCount(); i++)
    {
        properties.insert(tableWidget->item(i, 0)->text(), tableWidget->item(i, 1)->text());
    }

    qDebug()<<"properties tab "<< properties;
    emit propertiesChanged("changeElement", dynamic_cast<QGraphicsItem*>(getCurrentObject()), properties);

    updatePropertiesTable();
    setModified(false);
}

// Отменить изменения
void PropertiesTable::discardChanges()
{
    qDebug()<<"discard tab";
    updatePropertiesTable(); // Перезагружаем таблицу без изменений
    setModified(false);
}

void PropertiesTable::setModified(bool modified)
{
    qDebug()<<"modified tab "<< modified;
    isModified = modified;
    applyButton->setEnabled(modified);
    cancelButton->setEnabled(modified);
}
