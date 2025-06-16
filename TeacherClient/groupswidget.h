#pragma once

#include <QWidget>
#include <QHash>
#include <QStringList>

class QListWidget;
class QTableWidget;
class QPushButton;

class GroupsWidget : public QWidget {
    Q_OBJECT

public:
    explicit GroupsWidget(QWidget* parent = nullptr);

    void setGroupsFile(const QString& path)   { groupsFile = path; }
    void setStudentsFile(const QString& path) { studentsFile = path; }

signals:
    // При добавлении студентов вручную или из файла
    void addStudents(const QStringList& studentNames);

private slots:
    void onAddGroup();
    void onGroupChanged();
    void onAddStudent();
    void onImportStudents();
    void onAssignToGroup();
    void onRemoveFromGroup();

private:
    void setupUi();
    void loadGroups();
    void loadStudents();
    void saveGroups();
    void saveStudentAssignments();
    QString selectedGroupId() const;
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
};
