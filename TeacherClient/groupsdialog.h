// groupsdialog.h
#pragma once

#include <QDialog>
#include <QHash>
#include <QStringList>

class QListWidget;
class QTableWidget;
class QPushButton;

class GroupsDialog : public QDialog {
    Q_OBJECT

public:
    explicit GroupsDialog(QWidget* parent = nullptr);
    ~GroupsDialog() override;

    // Задать файлы для загрузки/сохранения
    void setGroupsFile(const QString& path)   { groupsFile = path; }
    void setStudentsFile(const QString& path) { studentsFile = path; }

    // Загрузить из файлов (вызывать до exec())
    void loadGroups();
    void loadStudents();

signals:
    // При ручном добавлении студентов (в таблице или импортом)
    void addStudents(const QStringList& studentNames);
    // При нажатии «Применить» — список ФИО студентов выбранной группы
    void groupStudentNames(const QStringList& names, QString group);

private slots:
    void onAddGroup();
    void onGroupChanged();
    void onAddStudent();
    void onImportStudents();
    void onAssignToGroup();
    void onRemoveFromGroup();
    void onApply();
    void onCancel();

private:
    void setupUi();
    void saveGroups();
    void saveStudentAssignments();
    QString selectedGroupId() const;
    QStringList studentsInGroupNames() const;
    void refreshTables();

    QString groupsFile       = "groups.json";
    QString studentsFile     = "students.json";

    QHash<QString, QString> groups;           // groupId → groupName
    QHash<QString, QString> studentGroupMap;  // studentHash → groupId or "-"
    QHash<QString, QString> studentNameMap;   // studentHash → name

    // UI
    QListWidget*  groupsList       = nullptr;
    QTableWidget* assignedTable    = nullptr;
    QTableWidget* unassignedTable  = nullptr;
    QPushButton*  addGroupBtn      = nullptr;
    QPushButton*  addStudentBtn    = nullptr;
    QPushButton*  importStudentsBtn= nullptr;
    QPushButton*  assignBtn        = nullptr;
    QPushButton*  removeBtn        = nullptr;
    QPushButton*  applyBtn         = nullptr;
    QPushButton*  cancelBtn        = nullptr;

    QString currGroupName;

};
