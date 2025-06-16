#ifndef STUDENTSELECTIONWIDGET_H
#define STUDENTSELECTIONWIDGET_H

#include <QWidget>
#include <QHash>
#include <QStringList>

class QListWidget;
class QToolButton;
class QCheckBox;
class QPushButton;

class StudentSelectionWidget : public QWidget {
    Q_OBJECT

public:
    explicit StudentSelectionWidget(QWidget* parent = nullptr);

private slots:
    // группы
    void onAddGroup();
    void onDeleteGroup();
    void onGroupChanged();
    void onSelectAllGroupsToggled(bool);
    // студенты
    void onAddStudent();
    void onImportStudents();
    void onSelectAllStudentsToggled(bool);
    // перенос
    void onAddSelected();
    void onRemoveSelected();
    //void onGroupsSelectionChanged();

private:
    void setupUi();
    void loadGroups();
    void loadStudents();
    void saveGroups();
    void saveStudentAssignments();
    void refreshStudents();

    QString groupsFile   = "groups.json";
    QString studentsFile = "students.json";

    QHash<QString, QString> groups;           // id → name
    QHash<QString, QString> studentGroupMap;  // hash → groupId or "-"
    QHash<QString, QString> studentNameMap;   // hash → name

    // UI
    QListWidget* groupListWidget       = nullptr;
    QToolButton* groupActionsBtn       = nullptr;
    QCheckBox*   selectAllGroups       = nullptr;

    QListWidget* availableStudentsList = nullptr;
    QToolButton* studentActionsBtn     = nullptr;
    QCheckBox*   selectAllStudents     = nullptr;

    QPushButton* addButton             = nullptr;
    QPushButton* removeButton          = nullptr;
    QListWidget* selectedStudentsList  = nullptr;

    QString selectedGroupId() const;
};
#endif // STUDENTSELECTIONWIDGET_H
