// variantswidget.cpp
#include "variantswidget.h"
#include "taskdialog.h"
#include <QListWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QColor>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

VariantsWidget::VariantsWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    loadFromFile("variants.json");
}

void VariantsWidget::setupUi() {
    taskListWidget = new QListWidget;
    variantListWidget = new QListWidget;
    descriptionTextEdit = new QTextEdit;
    descriptionTextEdit->setReadOnly(true);

    taskListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    taskListWidget->setAlternatingRowColors(false);

    taskActions = new QToolButton;
    taskActions->setText("Действия   ");
    QMenu* taskMenu = new QMenu(taskActions);
    taskMenu->addAction("Создать задание", this, &VariantsWidget::onAddTask);
    taskMenu->addAction("Изменить задание", this, &VariantsWidget::onEditTask);
    taskMenu->addAction("Удалить задание", this, &VariantsWidget::onRemoveTask);
    taskActions->setMenu(taskMenu);
    taskActions->setPopupMode(QToolButton::InstantPopup);
    taskActions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    taskActions->setStyleSheet("QToolButton::menu-indicator { subcontrol-position: right }");

    variantActions = new QToolButton;
    variantActions->setText("Действия   ");
    QMenu* variantMenu = new QMenu(variantActions);
    variantMenu->addAction("Создать вариант", this, &VariantsWidget::onAddVariant);
    variantMenu->addAction("Удалить вариант", this, &VariantsWidget::onRemoveVariant);
    variantActions->setMenu(variantMenu);
    variantActions->setPopupMode(QToolButton::InstantPopup);
    variantActions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    variantActions->setStyleSheet("QToolButton::menu-indicator { subcontrol-position: right }");

    auto* mainLayout = new QHBoxLayout(this);

    auto* taskLayout = new QVBoxLayout;
    auto* taskMenuLayout = new QHBoxLayout;
    taskMenuLayout->addWidget(new QLabel("Задания"));
    taskMenuLayout->addWidget(taskActions);
    taskLayout->addLayout(taskMenuLayout);
    taskLayout->addWidget(taskListWidget);

    auto* variantLayout = new QVBoxLayout;
    auto* variantMenuLayout = new QHBoxLayout;
    variantMenuLayout->addWidget(new QLabel("Варианты"));
    variantMenuLayout->addWidget(variantActions);
    variantLayout->addLayout(variantMenuLayout);
    variantLayout->addWidget(variantListWidget);

    auto* descriptionLayout = new QVBoxLayout;
    descriptionLayout->addWidget(new QLabel("Описание"));
    descriptionLayout->addWidget(descriptionTextEdit);

    mainLayout->addLayout(taskLayout, 2);
    mainLayout->addLayout(variantLayout, 1);
    mainLayout->addLayout(descriptionLayout, 2);

    connect(taskListWidget, &QListWidget::currentRowChanged, this, &VariantsWidget::onTaskSelected);
    connect(variantListWidget, &QListWidget::currentRowChanged, this, &VariantsWidget::onVariantSelected);
}

void VariantsWidget::onAddTask() {
    TaskDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Task t;
        t.id = nextTaskId();
        t.topology = dialog.selectedTopology();
        t.parameters = dialog.parameters();
        t.scores = dialog.subtaskScores();
        tasks.append(t);
        refreshTaskList();
        saveToFile("variants.json");
    }
}

void VariantsWidget::onEditTask() {
    int row = taskListWidget->currentRow();
    if (row < 0 || row >= tasks.size()) return;

    Task& t = tasks[row];
    TaskDialog dialog(this);
    dialog.setTask(t.topology, t.parameters);

    if (dialog.exec() == QDialog::Accepted) {
        t.topology = dialog.selectedTopology();
        t.parameters = dialog.parameters();
        t.scores = dialog.subtaskScores();
        refreshTaskList();
        updateDescription();
        saveToFile("variants.json");
    }
}

void VariantsWidget::onRemoveTask() {
    int row = taskListWidget->currentRow();
    if (row < 0 || row >= tasks.size()) return;

    int taskId = tasks[row].id;
    tasks.remove(row);

    for (Variant& v : variants)
        v.taskIds.removeAll(taskId);

    refreshTaskList();
    refreshVariantList();
    updateDescription();
    saveToFile("variants.json");
}

void VariantsWidget::onAddVariant() {
    QList<QListWidgetItem*> selected;
    for (int i = 0; i < taskListWidget->count(); ++i) {
        QListWidgetItem* item = taskListWidget->item(i);
        if (item->checkState() == Qt::Checked)
            selected.append(item);
    }
    if (selected.size() != 5) {
        QMessageBox::warning(this, "Ошибка", "Необходимо выбрать ровно 5 заданий для варианта.");
        return;
    }

    Variant variant;
    variant.id = nextVariantId();
    for (QListWidgetItem* item : selected) {
        int row = taskListWidget->row(item);
        variant.taskIds.append(tasks[row].id);
    }

    variants.append(variant);
    refreshVariantList();
    saveToFile("variants.json");
}

void VariantsWidget::onRemoveVariant() {
    int row = variantListWidget->currentRow();
    if (row < 0 || row >= variants.size()) return;
    variants.remove(row);
    refreshVariantList();
    updateDescription();
    saveToFile("variants.json");
}

void VariantsWidget::onTaskSelected() {
    updateDescription();
}

void VariantsWidget::onVariantSelected() {
    updateDescription();
}
void VariantsWidget::refreshTaskList() {
    taskListWidget->clear();
    for (int i = 0; i < tasks.size(); ++i) {
        const Task& task = tasks[i];
        QString text = QString("Задание №%1\nТопология: %2").arg(task.id).arg(task.topology);

        text += QString("\nПараметры:");

        for (auto it = task.parameters.begin(); it != task.parameters.end(); ++it)
            text += QString("\n%1: %2").arg(it.key()).arg(it.value());

        auto* item = new QListWidgetItem(text);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);

        QColor bg = (i % 2 == 0) ? QColor("#ffffff") : QColor("#f0f0f0");
        item->setBackground(bg);

        taskListWidget->addItem(item);
    }
}

void VariantsWidget::refreshVariantList() {
    variantListWidget->clear();
    for (const Variant& variant : variants) {
        variantListWidget->addItem(QString("№%1").arg(variant.id));
    }
}

void VariantsWidget::saveToFile(const QString& path) {
    QJsonArray tasksArray;
    for (const Task& t : tasks) {
        QJsonObject obj;
        obj["id"] = t.id;
        obj["topology"] = t.topology;
        QJsonObject paramObj;
        for (auto it = t.parameters.begin(); it != t.parameters.end(); ++it)
            paramObj[it.key()] = it.value();
        obj["parameters"] = paramObj;

        QJsonObject scoreObj;
        for (auto it = t.scores.begin(); it != t.scores.end(); ++it) {
            QJsonObject sub;
            sub["success"] = it.value().successPoints;
            sub["failure"] = it.value().failurePenalty;
            scoreObj[it.key()] = sub;
        }
        obj["scores"] = scoreObj;

        tasksArray.append(obj);
    }

    QJsonArray variantsArray;
    for (const Variant& v : variants) {
        QJsonObject obj;
        obj["id"] = v.id;
        QJsonArray taskIds;
        for (int tid : v.taskIds)
            taskIds.append(tid);
        obj["taskIds"] = taskIds;
        variantsArray.append(obj);
    }

    QJsonObject root;
    root["tasks"] = tasksArray;
    root["variants"] = variantsArray;

    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson());
        f.close();
    }
}

// внутри loadFromFile
void VariantsWidget::loadFromFile(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray data = f.readAll();
    f.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    tasks.clear();
    variants.clear();

    for (const QJsonValue& v : root["tasks"].toArray()) {
        QJsonObject o = v.toObject();
        Task t;
        t.id = o["id"].toInt();
        t.topology = o["topology"].toString();
        QJsonObject p = o["parameters"].toObject();
        for (auto it = p.begin(); it != p.end(); ++it)
            t.parameters[it.key()] = it.value().toInt();

        QJsonObject s = o["scores"].toObject();
        for (auto it = s.begin(); it != s.end(); ++it) {
            QJsonObject sub = it.value().toObject();
            SubtaskScore score;
            score.successPoints = sub["success"].toInt();
            score.failurePenalty = sub["failure"].toInt();
            t.scores[it.key()] = score;
        }

        tasks.append(t);
    }

    for (const QJsonValue& v : root["variants"].toArray()) {
        QJsonObject o = v.toObject();
        Variant var;
        var.id = o["id"].toInt();
        for (const QJsonValue& tid : o["taskIds"].toArray())
            var.taskIds.append(tid.toInt());
        variants.append(var);
    }

    refreshTaskList();
    refreshVariantList();
}


void VariantsWidget::updateDescription() {
    if (taskListWidget->hasFocus()) {
        int row = taskListWidget->currentRow();
        if (row >= 0 && row < tasks.size()) {
            const Task& t = tasks[row];
            QString desc = QString("Задание №%1\nТопология: %2\n").arg(t.id).arg(t.topology);
            for (auto it = t.parameters.begin(); it != t.parameters.end(); ++it) {
                desc += QString("%1: %2\n").arg(it.key(), QString::number(it.value()));
            }

            desc += QString("\nБаллы:\n");
            QStringList ordered = {"Построить схему", "Построить маршрут", "Выполнить блокировки"};
            for (const QString& subtask : ordered) {
                if (t.scores.contains(subtask)) {
                    const SubtaskScore& s = t.scores[subtask];
                    desc += QString("%1: +%2, %3\n").arg(subtask).arg(s.successPoints).arg(s.failurePenalty);
                }
            }

            descriptionTextEdit->setText(desc);
        }
    } else if (variantListWidget->hasFocus()) {
        int row = variantListWidget->currentRow();
        if (row >= 0 && row < variants.size()) {
            const Variant& v = variants[row];
            QString desc = QString("Вариант №%1\n\n").arg(v.id);
            int i = 1;
            for (int taskId : v.taskIds) {
                auto it = std::find_if(tasks.begin(), tasks.end(), [&](const Task& t) { return t.id == taskId; });
                if (it != tasks.end()) {
                    desc += QString("Задание №%1 (%2)\nТопология: %3\n")
                                .arg(i++)
                                .arg(it->id)
                                .arg(it->topology);

                    desc += QString("\nПараметры:\n");
                    for (auto p = it->parameters.begin(); p != it->parameters.end(); ++p) {
                        desc += QString("%1: %2\n").arg(p.key()).arg(p.value());
                    }

                    desc += QString("\nБаллы:\n");
                    QStringList ordered = {"Построить схему", "Построить маршрут", "Выполнить блокировки"};
                    for (const QString& subtask : ordered) {
                        if (it->scores.contains(subtask)) {
                            const SubtaskScore& s = it->scores[subtask];
                            desc += QString("%1: +%2, %3\n").arg(subtask).arg(s.successPoints).arg(s.failurePenalty);
                        }
                    }

                    desc += QString("\n------------------------------------\n");

                    desc += "\n";
                }
            }
            descriptionTextEdit->setText(desc);
        }
    } else {
        descriptionTextEdit->clear();
    }
}



int VariantsWidget::nextTaskId() const {
    int maxId = 0;
    for (const Task& t : tasks) maxId = std::max(maxId, t.id);
    return maxId + 1;
}

int VariantsWidget::nextVariantId() const {
    int maxId = 0;
    for (const Variant& v : variants) maxId = std::max(maxId, v.id);
    return maxId + 1;
}
