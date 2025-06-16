// taskdialog.cpp
#include "taskdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGroupBox>
#include <QStyle>
#include <QApplication>

TaskDialog::TaskDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Новое задание");
    resize(400, 500);

    auto* layout = new QVBoxLayout(this);
    topologyBox = new QComboBox(this);
    topologyBox->addItems({"Баньян", "Омега", "Бенеша", "Дельта", "Клоша"});
    connect(topologyBox, &QComboBox::currentTextChanged, this, &TaskDialog::onTopologyChanged);

    layout->addWidget(new QLabel("Топология:"));
    layout->addWidget(topologyBox);
    layout->addWidget(new QLabel("Параметры:"));
    paramForm = new QFormLayout;
    layout->addLayout(paramForm);

    layout->addWidget(new QLabel("Баллы:"));
    scoreForm = new QFormLayout;

    QStyle* style = QApplication::style();
    QIcon checkIcon = style->standardIcon(QStyle::SP_DialogApplyButton);
    QIcon crossIcon = style->standardIcon(QStyle::SP_DialogCancelButton);

    for (const QString& subtask : {"Построить схему", "Построить маршрут", "Выполнить блокировки"}) {
        QSpinBox* success = new QSpinBox;
        QSpinBox* fail = new QSpinBox;
        success->setRange(-100, 100);
        fail->setRange(-100, 100);
        success->setValue(10);
        fail->setValue(0);

        auto* row = new QWidget;
        auto* rowLayout = new QHBoxLayout(row);
        QLabel* successIcon = new QLabel;
        successIcon->setPixmap(checkIcon.pixmap(16, 16));
        QLabel* failIcon = new QLabel;
        failIcon->setPixmap(crossIcon.pixmap(16, 16));

        rowLayout->addWidget(successIcon);
        rowLayout->addWidget(success);
        rowLayout->addSpacing(10);
        rowLayout->addWidget(failIcon);
        rowLayout->addWidget(fail);
        rowLayout->addStretch();

        scoreForm->addRow(subtask + ":", row);
        scoreFields[subtask] = {success, fail};
    }

    layout->addLayout(scoreForm);

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    onTopologyChanged(topologyBox->currentText());
}


QString TaskDialog::selectedTopology() const {
    return topologyBox->currentText();
}

QMap<QString, int> TaskDialog::parameters() const {
    QMap<QString, int> result;
    for (auto it = inputFields.begin(); it != inputFields.end(); ++it) {
        result[it.key()] = it.value()->value();
    }
    return result;
}

QMap<QString, SubtaskScore> TaskDialog::subtaskScores() const {
    QMap<QString, SubtaskScore> result;
    for (auto it = scoreFields.begin(); it != scoreFields.end(); ++it) {
        SubtaskScore s;
        s.successPoints = it.value().first->value();
        s.failurePenalty = it.value().second->value();
        result[it.key()] = s;
    }
    return result;
}

void TaskDialog::setTask(const QString& topology, const QMap<QString, int>& params) {
    int index = topologyBox->findText(topology);
    if (index >= 0) {
        topologyBox->setCurrentIndex(index);
    }
    for (auto it = inputFields.begin(); it != inputFields.end(); ++it) {
        if (params.contains(it.key())) {
            it.value()->setValue(params[it.key()]);
        }
    }
}

void TaskDialog::onTopologyChanged(const QString& topology) {
    QLayoutItem* child;
    while ((child = paramForm->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    inputFields.clear();

    QStringList params;
    if (topology == "Баньян" || topology == "Омега" || topology == "Бенеша") {
        params << "Количество входов" << "Количество выходов";
    } else if (topology == "Дельта") {
        params << "Входов кроссбара" << "Выходов кроссбара" << "Ступеней";
    } else if (topology == "Клоша") {
        params << "Кроссбаров во входной ступени"
               << "Кроссбаров в промежуточной ступени"
               << "Кроссбаров в выходной ступени"
               << "Входов кроссбаров во входной ступени"
               << "Выходов кроссбаров в выходной ступени";
    }

    for (const QString& param : params) {
        QSpinBox* spin = new QSpinBox;
        spin->setMinimum(1);
        spin->setMaximum(100);
        paramForm->addRow(param + ":", spin);
        inputFields[param] = spin;
    }
}
