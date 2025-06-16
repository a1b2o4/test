// groupswidget.cpp
#include "groupswidget.h"
#include <QListWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QLabel>

GroupsWidget::GroupsWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    loadGroups();
    loadStudents();

    connect(groupsList, &QListWidget::currentTextChanged, this, &GroupsWidget::onGroupChanged);
    connect(assignBtn, &QPushButton::clicked, this, &GroupsWidget::onAssignToGroup);
    connect(removeBtn, &QPushButton::clicked, this, &GroupsWidget::onRemoveFromGroup);

    connect(this, &GroupsWidget::addStudents, this, [this](const QStringList& names) {
        QString gid = selectedGroupId();
        if (gid.isEmpty()) {
            QMessageBox::warning(this, tr("Группа не выбрана"), tr("Сначала выберите группу."));
            return;
        }

        for (const QString& name : names) {
            QByteArray b = name.toUtf8();
            QString hash = QCryptographicHash::hash(b, QCryptographicHash::Sha256).toHex();
            if (!studentNameMap.contains(hash)) {
                studentNameMap[hash] = name;
                studentGroupMap[hash] = gid;
            }
        }
        saveStudentAssignments();
        refreshTables();
    });
}

void GroupsWidget::setupUi() {
    groupsList = new QListWidget;
    assignedTable = new QTableWidget(0, 1);
    unassignedTable = new QTableWidget(0, 1);

    assignedTable->setHorizontalHeaderLabels({tr("Студенты в группе")});
    unassignedTable->setHorizontalHeaderLabels({tr("Нераспределённые студенты")});

    assignBtn = new QPushButton(tr("<<"));
    removeBtn = new QPushButton(tr(">>"));

    QToolButton* groupActions = new QToolButton;
    groupActions->setText(tr("Действия   "));
    QMenu* groupMenu = new QMenu(groupActions);
    groupMenu->addAction(tr("Добавить группу"), this, &GroupsWidget::onAddGroup);
    groupMenu->addAction(tr("Удалить группу"), this, [this]() {
        QString gid = selectedGroupId();
        if (gid.isEmpty()) return;

        const QString groupName = groups.value(gid);
        QMessageBox::StandardButton btn = QMessageBox::question(
            this,
            tr("Удалить группу"),
            tr("Удалить студентов группы \"%1\" вместе с группой?\nНажмите Нет, чтобы перенести студентов в нераспределённые.").arg(groupName),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (btn == QMessageBox::Cancel) return;

        // Удалить студентов или перенести
        for (auto it = studentGroupMap.begin(); it != studentGroupMap.end();) {
            if (it.value() == gid) {
                if (btn == QMessageBox::Yes) {
                    studentNameMap.remove(it.key());
                    it = studentGroupMap.erase(it);
                    continue;
                } else {
                    it.value() = "-";
                }
            }
            ++it;
        }

        groups.remove(gid);
        saveGroups();
        saveStudentAssignments();
        loadGroups();

        // Сделать последнюю группу активной
        if (groupsList->count() > 0)
            groupsList->setCurrentRow(groupsList->count() - 1);

        refreshTables();
    });
    groupActions->setMenu(groupMenu);
    groupActions->setPopupMode(QToolButton::InstantPopup);
    groupActions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    groupActions->setStyleSheet("QToolButton::menu-indicator { subcontrol-position: right }");

    QToolButton* studentActions = new QToolButton;
    studentActions->setText(tr("Действия   "));
    QMenu* studentMenu = new QMenu(studentActions);
    studentMenu->addAction(tr("Добавить студента"), this, &GroupsWidget::onAddStudent);
    studentMenu->addAction(tr("Загрузить из файла"), this, &GroupsWidget::onImportStudents);
    studentMenu->addAction(tr("Удалить выбранных"), this, [this]() {
        auto selected = assignedTable->selectionModel()->selectedRows();
        for (const QModelIndex& index : selected) {
            QString hash = assignedTable->item(index.row(), 0)->data(Qt::UserRole).toString();
            studentNameMap.remove(hash);
            studentGroupMap.remove(hash);
        }
        saveStudentAssignments();
        refreshTables();
    });
    studentActions->setMenu(studentMenu);
    studentActions->setPopupMode(QToolButton::InstantPopup);
    studentActions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    studentActions->setStyleSheet("QToolButton::menu-indicator { subcontrol-position: right }");

    auto* mainLayout = new QHBoxLayout(this);

    auto* groupMenuLayout = new QHBoxLayout;
    auto* groupLayout = new QVBoxLayout;
    groupMenuLayout->addWidget(new QLabel(tr("Группы студентов")));
    groupMenuLayout->addWidget(groupActions);
    groupLayout->addItem(groupMenuLayout);
    groupLayout->addWidget(groupsList);

    auto* centerMenuLayout = new QHBoxLayout;
    auto* centerLayout = new QVBoxLayout;
    centerMenuLayout->addWidget(new QLabel(tr("Студенты группы")));
    centerMenuLayout->addWidget(studentActions);
    centerLayout->addItem(centerMenuLayout);
    centerLayout->addWidget(assignedTable);

    auto* moveButtonsLayout = new QVBoxLayout;
    moveButtonsLayout->addStretch();
    moveButtonsLayout->addWidget(removeBtn);
    moveButtonsLayout->addWidget(assignBtn);
    moveButtonsLayout->addStretch();

    auto* unassignedLayout = new QVBoxLayout;
    unassignedLayout->addWidget(new QLabel(tr("Нераспределённые студенты")));
    unassignedLayout->addWidget(unassignedTable);

    assignedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    assignedTable->setSelectionMode(QAbstractItemView::MultiSelection);
    unassignedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    unassignedTable->setSelectionMode(QAbstractItemView::MultiSelection);

    mainLayout->addLayout(groupLayout, 1);
    mainLayout->addLayout(centerLayout, 2);
    mainLayout->addLayout(moveButtonsLayout);
    mainLayout->addLayout(unassignedLayout, 1);
}


void GroupsWidget::loadGroups() {
    groups.clear();
    groupsList->clear();

    QFile f(groupsFile);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    for (const QJsonValue& v : arr) {
        QJsonObject obj = v.toObject();
        QString id = obj["id"].toString();
        QString name = obj["name"].toString();
        groups[id] = name;
        groupsList->addItem(name);
    }
}

void GroupsWidget::loadStudents() {
    studentGroupMap.clear();
    studentNameMap.clear();

    QFile f(studentsFile);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    for (const QJsonValue& v : arr) {
        QJsonObject obj = v.toObject();
        QString hash = obj["hash"].toString();
        QString name = obj["name"].toString();
        QString gid = obj["groupId"].toString("-");
        studentGroupMap[hash] = gid;
        studentNameMap[hash] = name;
    }
    refreshTables();
}

void GroupsWidget::saveGroups() {
    QJsonArray arr;
    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        QJsonObject o;
        o["id"] = it.key();
        o["name"] = it.value();
        arr.append(o);
    }
    QFile f(groupsFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
    }
}

void GroupsWidget::saveStudentAssignments() {
    QJsonArray arr;
    for (auto it = studentGroupMap.constBegin(); it != studentGroupMap.constEnd(); ++it) {
        QJsonObject o;
        o["hash"] = it.key();
        o["name"] = studentNameMap.value(it.key());
        o["groupId"] = it.value();
        arr.append(o);
    }
    QFile f(studentsFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
    }
}

QString GroupsWidget::selectedGroupId() const {
    auto* item = groupsList->currentItem();
    if (!item) return QString();
    QString name = item->text();
    return groups.key(name, QString());
}

void GroupsWidget::refreshTables() {
    assignedTable->clearContents();
    assignedTable->setRowCount(0);
    unassignedTable->clearContents();
    unassignedTable->setRowCount(0);

    QString gid = selectedGroupId();
    int a = 0, u = 0;
    for (auto it = studentGroupMap.constBegin(); it != studentGroupMap.constEnd(); ++it) {
        const QString& hash = it.key();
        const QString& grp = it.value();
        const QString& name = studentNameMap.value(hash);

        auto* item = new QTableWidgetItem(name);
        item->setData(Qt::UserRole, hash);

        if (grp == gid) {
            assignedTable->insertRow(a);
            assignedTable->setItem(a++, 0, item);
        } else if (grp == "-") {
            unassignedTable->insertRow(u);
            unassignedTable->setItem(u++, 0, item);
        }
    }
}

void GroupsWidget::onAddGroup() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Новая группа"), tr("Название:"), QLineEdit::Normal, {}, &ok);
    if (!ok || name.isEmpty()) return;
    QByteArray b = name.toUtf8();
    QString id = QCryptographicHash::hash(b, QCryptographicHash::Sha256).toHex();
    groups[id] = name;
    saveGroups();
    loadGroups();
}

void GroupsWidget::onGroupChanged() {
    refreshTables();
}

void GroupsWidget::onAddStudent() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Новый студент"), tr("ФИО:"), QLineEdit::Normal, {}, &ok);
    if (!ok || name.isEmpty()) return;
    emit addStudents(QStringList{name});
}

void GroupsWidget::onImportStudents() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Выберите файл со студентами"), {}, tr("Text or CSV (*.txt *.csv)"));
    if (fn.isEmpty()) return;

    QStringList names;
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty())
            names << line;
    }
    emit addStudents(names);
}

void GroupsWidget::onAssignToGroup() {
    QString gid = selectedGroupId();
    for (auto idx : unassignedTable->selectionModel()->selectedRows()) {
        QString hash = unassignedTable->item(idx.row(), 0)->data(Qt::UserRole).toString();
        studentGroupMap[hash] = gid;
    }
    saveStudentAssignments();
    refreshTables();
}

void GroupsWidget::onRemoveFromGroup() {
    for (auto idx : assignedTable->selectionModel()->selectedRows()) {
        QString hash = assignedTable->item(idx.row(), 0)->data(Qt::UserRole).toString();
        studentGroupMap[hash] = "-";
    }
    saveStudentAssignments();
    refreshTables();
}
