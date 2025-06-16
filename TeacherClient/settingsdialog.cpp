#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QHeaderView>

#include <QCheckBox>  // Добавляем для работы с чекбоксами

SettingsDialog::SettingsDialog(const QJsonArray& settings, QWidget* parent)
    : QDialog(parent), settings(settings) {

    setWindowTitle("Редактирование настроек");
    setMinimumSize(400, 300);

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels({"Название", "Значение"});
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    saveButton = new QPushButton("Сохранить", this);

    // Заполняем таблицу
    tableWidget->setRowCount(settings.size());
    for (int i = 0; i < settings.size(); ++i) {
        QJsonObject setting = settings[i].toObject();

        QString name = setting["name"].toString();
        QString type = setting["type"].toString();
        QVariant value = setting["value"].toVariant();

        // Название настройки
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        tableWidget->setItem(i, 0, nameItem);

        // Элемент для редактирования значения
        if (type == "int") {
            QSpinBox* spinBox = new QSpinBox();
            spinBox->setMinimum(0);
            spinBox->setMaximum(1000);
            spinBox->setValue(value.toInt());
            tableWidget->setCellWidget(i, 1, spinBox);
        }
        else if (type == "bool") {
            QCheckBox* checkBox = new QCheckBox();
            checkBox->setChecked(value.toBool());
            tableWidget->setCellWidget(i, 1, checkBox);
        }
        else if (type == "double") {
            QSpinBox* spinBox = new QSpinBox();
            spinBox->setMinimum(0);
            spinBox->setMaximum(1000);
            spinBox->setValue(value.toDouble());
            tableWidget->setCellWidget(i, 1, spinBox);
        }
        else {
            QTableWidgetItem* valueItem = new QTableWidgetItem(value.toString());
            tableWidget->setItem(i, 1, valueItem);
        }
    }

    // Разметка
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tableWidget);
    mainLayout->addWidget(saveButton);
    setLayout(mainLayout);

    // Сигнал на кнопку сохранения
    connect(saveButton, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
}

void SettingsDialog::onSaveClicked() {
    for (int i = 0; i < settings.size(); ++i) {
        QJsonObject setting = settings[i].toObject();
        QString type = setting["type"].toString();

        if (type == "int") {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(tableWidget->cellWidget(i, 1));
            if (spinBox) setting["value"] = spinBox->value();
        }
        else if (type == "bool") {
            QCheckBox* checkBox = qobject_cast<QCheckBox*>(tableWidget->cellWidget(i, 1));
            if (checkBox) setting["value"] = checkBox->isChecked();
        }
        else if (type == "double") {
            QSpinBox* spinBox = qobject_cast<QSpinBox*>(tableWidget->cellWidget(i, 1));
            if (spinBox) setting["value"] = spinBox->value();
        }
        else {
            QTableWidgetItem* valueItem = tableWidget->item(i, 1);
            if (valueItem) setting["value"] = valueItem->text();
        }

        settings[i] = setting;
    }
    qDebug() << "settings " << settings;

    accept(); // Закрываем диалог
}


void SettingsDialog::updateSettings(const QJsonArray& newSettings) {
    settings = newSettings;
    tableWidget->clearContents();
    tableWidget->setRowCount(settings.size());

    for (int i = 0; i < settings.size(); ++i) {
        QJsonObject setting = settings[i].toObject();

        QString name = setting["name"].toString();
        QString type = setting["type"].toString();
        QVariant value = setting["value"].toVariant();

        // Название настройки
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        tableWidget->setItem(i, 0, nameItem);

        // Элемент для редактирования значения
        if (type == "int") {
            QSpinBox* spinBox = new QSpinBox();
            spinBox->setMinimum(0);
            spinBox->setMaximum(1000);
            spinBox->setValue(value.toInt());
            tableWidget->setCellWidget(i, 1, spinBox);
        }
        else if (type == "bool") {
            QCheckBox* checkBox = new QCheckBox();
            checkBox->setChecked(value.toBool());
            tableWidget->setCellWidget(i, 1, checkBox);
        }
        else if (type == "double") {
            QSpinBox* spinBox = new QSpinBox();
            spinBox->setMinimum(0);
            spinBox->setMaximum(1000);
            spinBox->setValue(value.toInt());
            tableWidget->setCellWidget(i, 1, spinBox);
        }
        else {
            QTableWidgetItem* valueItem = new QTableWidgetItem(value.toString());
            tableWidget->setItem(i, 1, valueItem);
        }
    }
}

QJsonArray SettingsDialog::getUpdatedSettings() const {
    return settings;
}
