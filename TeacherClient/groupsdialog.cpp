// groupsdialog.cpp
#include "groupsdialog.h"
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

GroupsDialog::GroupsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();

    connect(addGroupBtn,        &QPushButton::clicked, this, &GroupsDialog::onAddGroup);
    connect(groupsList,         &QListWidget::currentTextChanged, this, &GroupsDialog::onGroupChanged);
    connect(addStudentBtn,      &QPushButton::clicked, this, &GroupsDialog::onAddStudent);
    connect(importStudentsBtn,  &QPushButton::clicked, this, &GroupsDialog::onImportStudents);
    connect(assignBtn,          &QPushButton::clicked, this, &GroupsDialog::onAssignToGroup);
    connect(removeBtn,          &QPushButton::clicked, this, &GroupsDialog::onRemoveFromGroup);
    connect(applyBtn,           &QPushButton::clicked, this, &GroupsDialog::onApply);
    connect(cancelBtn,          &QPushButton::clicked, this, &GroupsDialog::onCancel);

    assignedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    assignedTable->setSelectionMode(QAbstractItemView::MultiSelection);
    unassignedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    unassignedTable->setSelectionMode(QAbstractItemView::MultiSelection);

    connect(this, &GroupsDialog::addStudents, this, [this](const QStringList& names) {
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
                studentGroupMap[hash] = gid;  // сразу в выбранную группу
            }
        }

        saveStudentAssignments();
        refreshTables();
    });
}

GroupsDialog::~GroupsDialog() = default;

void GroupsDialog::setupUi()
{
    groupsList        = new QListWidget;
    assignedTable     = new QTableWidget(0, 1);
    unassignedTable   = new QTableWidget(0, 1);
    addGroupBtn       = new QPushButton(tr("Добавить группу"));
    addStudentBtn     = new QPushButton(tr("Добавить студента"));
    importStudentsBtn = new QPushButton(tr("Загрузить из файла"));
    assignBtn         = new QPushButton(tr("<< Добавить"));
    removeBtn         = new QPushButton(tr("Удалить >>"));
    applyBtn          = new QPushButton(tr("Применить"));
    cancelBtn         = new QPushButton(tr("Отмена"));

    assignedTable->setColumnCount(1);
    assignedTable->setHorizontalHeaderLabels(QStringList{tr("Студенты в группе")});

    unassignedTable->setColumnCount(1);
    unassignedTable->setHorizontalHeaderLabels(QStringList{tr("Нераспределённые")});

    auto* mainLay = new QHBoxLayout(this);
    auto* leftLay = new QVBoxLayout;
    auto* midLay  = new QVBoxLayout;
    auto* rightLay= new QVBoxLayout;

    leftLay->addWidget(groupsList);
    leftLay->addWidget(addGroupBtn);
    leftLay->addWidget(addStudentBtn);
    leftLay->addWidget(importStudentsBtn);

    midLay->addWidget(assignedTable);
    midLay->addWidget(assignBtn);
    midLay->addWidget(removeBtn);
    midLay->addWidget(unassignedTable);

    rightLay->addStretch();
    rightLay->addWidget(applyBtn);
    rightLay->addWidget(cancelBtn);

    mainLay->addLayout(leftLay);
    mainLay->addLayout(midLay);
    mainLay->addLayout(rightLay);
}

void GroupsDialog::loadGroups()
{
    groups.clear();
    groupsList->clear();

    QFile f(groupsFile);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    for (const QJsonValue& v : arr) {
        QJsonObject obj = v.toObject();
        QString id   = obj["id"].toString();
        QString name = obj["name"].toString();
        groups[id] = name;
        groupsList->addItem(name);
    }
}

void GroupsDialog::loadStudents()
{
    studentGroupMap.clear();
    studentNameMap.clear();

    QFile f(studentsFile);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    for (const QJsonValue& v : arr) {
        QJsonObject obj = v.toObject();
        QString hash    = obj["hash"].toString();
        QString name    = obj["name"].toString();
        QString gid     = obj["groupId"].toString("-");
        studentGroupMap[hash] = gid;
        studentNameMap[hash]  = name;
    }
    refreshTables();
}

void GroupsDialog::saveGroups()
{
    QJsonArray arr;
    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        QJsonObject o;
        o["id"]   = it.key();
        o["name"] = it.value();
        arr.append(o);
    }
    QFile f(groupsFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
    }
}

void GroupsDialog::saveStudentAssignments()
{
    QJsonArray arr;
    for (auto it = studentGroupMap.constBegin(); it != studentGroupMap.constEnd(); ++it) {
        QJsonObject o;
        o["hash"]    = it.key();
        o["name"]    = studentNameMap.value(it.key());
        o["groupId"] = it.value();
        arr.append(o);
    }
    QFile f(studentsFile);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(arr).toJson());
    }
}

QString GroupsDialog::selectedGroupId() const
{
    auto* item = groupsList->currentItem();
    if (!item) return QString();
    QString name = item->text();
    return groups.key(name, QString());
}

QStringList GroupsDialog::studentsInGroupNames() const
{
    QStringList lst;
    QString gid = selectedGroupId();
    for (auto it = studentGroupMap.constBegin(); it != studentGroupMap.constEnd(); ++it) {
        if (it.value() == gid) {
            lst << studentNameMap.value(it.key());
        }
    }
    return lst;
}

void GroupsDialog::refreshTables()
{
    assignedTable->clearContents();
    assignedTable->setRowCount(0);
    unassignedTable->clearContents();
    unassignedTable->setRowCount(0);

    QString gid = selectedGroupId();
    int a = 0, u = 0;
    for (auto it = studentGroupMap.constBegin(); it != studentGroupMap.constEnd(); ++it) {
        const QString& hash = it.key();
        const QString& grp  = it.value();
        const QString& name = studentNameMap.value(hash);

        auto* item = new QTableWidgetItem(name);
        // Хэш сохраняем, но не показываем:
        item->setData(Qt::UserRole, hash);

        if (grp == gid) {
            assignedTable->insertRow(a);
            assignedTable->setItem(a++, 0, item);
            currGroupName = groups[gid];
        } else if (grp == "-") {
            unassignedTable->insertRow(u);
            unassignedTable->setItem(u++, 0, item);
        }
    }
}

void GroupsDialog::onAddGroup()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Новая группа"),
                                         tr("Название:"), QLineEdit::Normal, {}, &ok);
    if (!ok || name.isEmpty()) return;
    QByteArray b = name.toUtf8();
    QString id = QCryptographicHash::hash(b, QCryptographicHash::Sha256).toHex();
    groups[id] = name;
    saveGroups();
    loadGroups();
}

void GroupsDialog::onGroupChanged()
{
    refreshTables();
}

void GroupsDialog::onAddStudent()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Новый студент"),
                                         tr("ФИО:"), QLineEdit::Normal, {}, &ok);
    if (!ok || name.isEmpty()) return;
    emit addStudents(QStringList{name});
}

void GroupsDialog::onImportStudents()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Выберите файл со студентами"),
                                              {}, tr("Text or CSV (*.txt *.csv)"));
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

void GroupsDialog::onAssignToGroup()
{
    QString gid = selectedGroupId();
    for (auto idx : unassignedTable->selectionModel()->selectedRows()) {
        QString hash = unassignedTable->item(idx.row(), 0)->data(Qt::UserRole).toString();
        studentGroupMap[hash] = gid;
    }
    saveStudentAssignments();
    refreshTables();
}

void GroupsDialog::onRemoveFromGroup()
{
    for (auto idx : assignedTable->selectionModel()->selectedRows()) {
        QString hash = assignedTable->item(idx.row(), 0)->data(Qt::UserRole).toString();
        studentGroupMap[hash] = "-";
    }
    saveStudentAssignments();
    refreshTables();
}

void GroupsDialog::onApply()
{
    //emit addStudents(studentsInGroupNames());
    emit groupStudentNames(studentsInGroupNames(), currGroupName);

    accept();
}

void GroupsDialog::onCancel()
{
    reject();
}
