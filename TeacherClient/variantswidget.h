// variantswidget.h
#pragma once

#include <QWidget>
#include <QVector>
#include <QMap>

#include "types.h"

class QListWidget;
class QTextEdit;
class QToolButton;


struct Task {
    int id;
    QString topology;
    QMap<QString, int> parameters;
    QMap<QString, SubtaskScore> scores;
};


struct Variant {
    int id;
    QList<int> taskIds;
};

class VariantsWidget : public QWidget {
    Q_OBJECT

public:
    explicit VariantsWidget(QWidget* parent = nullptr);

private slots:
    void onAddTask();
    void onEditTask();
    void onRemoveTask();
    void onAddVariant();
    void onRemoveVariant();
    void onTaskSelected();
    void onVariantSelected();

private:
    void setupUi();
    void refreshTaskList();
    void refreshVariantList();
    void updateDescription();
    int nextTaskId() const;
    int nextVariantId() const;

    void saveToFile(const QString& path);
    void loadFromFile(const QString& path);

    QVector<Task> tasks;
    QVector<Variant> variants;

    QListWidget* taskListWidget = nullptr;
    QListWidget* variantListWidget = nullptr;
    QTextEdit* descriptionTextEdit = nullptr;
    QToolButton* taskActions = nullptr;
    QToolButton* variantActions = nullptr;
};
