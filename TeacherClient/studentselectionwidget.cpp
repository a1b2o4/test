#include "studentselectionwidget.h"
#include <QListWidget>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTextStream>
#include <QMessageBox>
#include <QCryptographicHash>

StudentSelectionWidget::StudentSelectionWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    loadGroups();
    loadStudents();
}

void StudentSelectionWidget::setupUi()
{
    // группы
    groupListWidget = new QListWidget;
    selectAllGroups = new QCheckBox("Выбрать все группы");

    QVBoxLayout* leftLay = new QVBoxLayout;
    leftLay->addWidget(groupActionsBtn);
    leftLay->addWidget(groupListWidget);
    leftLay->addWidget(selectAllGroups);

    connect(groupListWidget, &QListWidget::currentTextChanged, this, &StudentSelectionWidget::onGroupChanged);
    connect(selectAllGroups, &QCheckBox::toggled, this, &StudentSelectionWidget::onSelectAllGroupsToggled);

    // студенты по группам
    availableStudentsList = new QListWidget;
    studentActionsBtn = new QToolButton;

    selectAllStudents = new QCheckBox("Выбрать всех студентов");

    QVBoxLayout* midLay = new QVBoxLayout;
    midLay->addWidget(availableStudentsList);
    midLay->addWidget(selectAllStudents);

    connect(selectAllStudents, &QCheckBox::toggled, this, &StudentSelectionWidget::onSelectAllStudentsToggled);

    // кнопки переноса
    addButton = new QPushButton(">>");
    removeButton = new QPushButton("<<");
    QVBoxLayout* btnLay = new QVBoxLayout;
    btnLay->addStretch();
    btnLay->addWidget(addButton);
    btnLay->addWidget(removeButton);
    btnLay->addStretch();
    connect(addButton, &QPushButton::clicked, this, &StudentSelectionWidget::onAddSelected);
    connect(removeButton, &QPushButton::clicked, this, &StudentSelectionWidget::onRemoveSelected);

    // выбранные студенты
    selectedStudentsList = new QListWidget;
    QVBoxLayout* rightLay = new QVBoxLayout;
    rightLay->addWidget(selectedStudentsList);

    // общий
    auto* mainLay = new QHBoxLayout(this);
    mainLay->addLayout(leftLay, 1);
    mainLay->addLayout(midLay, 2);
    mainLay->addLayout(btnLay);
    mainLay->addLayout(rightLay, 2);
}

void StudentSelectionWidget::loadGroups()
{
    groups.clear();
    groupListWidget->clear();
    QFile f(groupsFile);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    for (auto v : arr.toVariantList()) {
        auto o = v.toJsonObject();
        QString id = o["id"].toString(), name = o["name"].toString();
        groups[id] = name;
        groupListWidget->addItem(name);
    }
}

void StudentSelectionWidget::loadStudents()
{
    studentGroupMap.clear();
    studentNameMap.clear();

    QFile f(studentsFile);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    for (auto v : arr.toVariantList()) {
        auto o = v.toJsonObject();
        QString hash = o["hash"].toString(),
            name = o["name"].toString(),
            gid  = o["groupId"].toString("-");
        studentGroupMap[hash] = gid;
        studentNameMap[hash]  = name;
    }
    refreshStudents();
}

void StudentSelectionWidget::saveGroups()
{
    QJsonArray arr;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        QJsonObject o; o["id"] = it.key(); o["name"] = it.value();
        arr.append(o);
    }
    QFile f(groupsFile);
    if (f.open(QIODevice::WriteOnly)) f.write(QJsonDocument(arr).toJson());
}

void StudentSelectionWidget::saveStudentAssignments()
{
    QJsonArray arr;
    for (auto it = studentGroupMap.begin(); it != studentGroupMap.end(); ++it) {
        QJsonObject o;
        o["hash"]    = it.key();
        o["name"]    = studentNameMap[it.key()];
        o["groupId"] = it.value();
        arr.append(o);
    }
    QFile f(studentsFile);
    if (f.open(QIODevice::WriteOnly)) f.write(QJsonDocument(arr).toJson());
}

QString StudentSelectionWidget::selectedGroupId() const
{
    auto it = groupListWidget->currentItem();
    if (!it) return {};
    QString name = it->text();
    return groups.key(name, {});
}

void StudentSelectionWidget::onAddGroup()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Новая группа", "Название:", QLineEdit::Normal, {}, &ok);
    if (!ok || name.isEmpty()) return;
    QByteArray b = name.toUtf8();
    QString id = QCryptographicHash::hash(b, QCryptographicHash::Sha256).toHex();
    groups[id] = name;
    saveGroups();
    loadGroups();
}

void StudentSelectionWidget::onDeleteGroup()
{
    QString gid = selectedGroupId();
    if (gid.isEmpty()) return;
    QString groupName = groups[gid];
    auto btn = QMessageBox::question(this, "Удалить группу",
                                     QString("Удалить группу \"%1\"?").arg(groupName),
                                     QMessageBox::Yes|QMessageBox::No);
    if (btn != QMessageBox::Yes) return;

    // сбросить студентов
    for (auto it = studentGroupMap.begin(); it != studentGroupMap.end(); ++it)
        if (it.value()==gid) it.value() = "-";

    groups.remove(gid);
    saveGroups();
    saveStudentAssignments();
    loadGroups();
    loadStudents();
}

void StudentSelectionWidget::onGroupChanged()
{
    refreshStudents();
}

void StudentSelectionWidget::refreshStudents()
{
    availableStudentsList->clear();
    QString gid = selectedGroupId();
    for (auto it = studentGroupMap.begin(); it != studentGroupMap.end(); ++it) {
        if (it.value() == gid) {
            auto* item = new QListWidgetItem(studentNameMap[it.key()]);
            item->setData(Qt::UserRole, it.key());
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            availableStudentsList->addItem(item);
        }
    }
}

void StudentSelectionWidget::onSelectAllGroupsToggled(bool checked)
{
    groupListWidget->clearSelection();
    for (int i = 0; i < groupListWidget->count(); ++i)
        groupListWidget->item(i)->setSelected(checked);
}

void StudentSelectionWidget::onAddStudent()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Новый студент", "ФИО:", QLineEdit::Normal, {}, &ok);
    if (!ok || name.isEmpty()) return;
    auto id = selectedGroupId();
    if (id.isEmpty()) {
        QMessageBox::warning(this,"Ошибка","Сначала выберите группу");
        return;
    }
    QByteArray b = name.toUtf8();
    QString hash = QCryptographicHash::hash(b, QCryptographicHash::Sha256).toHex();
    studentNameMap[hash] = name;
    studentGroupMap[hash] = id;
    saveStudentAssignments();
    refreshStudents();
}

void StudentSelectionWidget::onImportStudents()
{
    QString fn = QFileDialog::getOpenFileName(this, "Выберите файл", {}, "Text/CSV (*.txt *.csv)");
    if (fn.isEmpty()) return;
    QStringList names;
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) return;
    QTextStream in(&f);
    while(!in.atEnd()){
        auto line = in.readLine().trimmed();
        if (!line.isEmpty()) names<<line;
    }
    for(auto& name:names){
        QByteArray b=name.toUtf8();
        QString hash=QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex();
        if (!studentNameMap.contains(hash)){
            studentNameMap[hash]=name;
            studentGroupMap[hash]=selectedGroupId();
        }
    }
    saveStudentAssignments();
    refreshStudents();
}

void StudentSelectionWidget::onSelectAllStudentsToggled(bool checked)
{
    for(int i=0;i<availableStudentsList->count();++i)
        availableStudentsList->item(i)->setCheckState(checked?Qt::Checked:Qt::Unchecked);
}

void StudentSelectionWidget::onAddSelected()
{
    for(int i=0;i<availableStudentsList->count();++i){
        auto* item=availableStudentsList->item(i);
        if(item->checkState()==Qt::Checked){
            auto* copy=new QListWidgetItem(item->text(),selectedStudentsList);
            copy->setData(Qt::UserRole,item->data(Qt::UserRole));
        }
    }
}

void StudentSelectionWidget::onRemoveSelected()
{
    qDeleteAll(selectedStudentsList->selectedItems());
}
