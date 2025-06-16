#include <QMainWindow>
#include <QTcpSocket>
#include <QTableWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVBoxLayout>
#include <QJsonArray>

#include "settingsdialog.h"
#include "groupswidget.h"
#include "studentselectionwidget.h"

class studentInfo;
class QTextEdit;

class TeacherClient : public QMainWindow {
    Q_OBJECT

public:
    TeacherClient(QWidget* parent = nullptr);
    ~TeacherClient();
    void connectToServer(const QString& host, quint16 port);
    void sendRequest(const QString& requestType, const QJsonObject& data = QJsonObject(), bool wait = true);
    void handleResponse(const QByteArray& responseData);

    QStringList getVariants() { return variants; }


public slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private slots:
    /*void sendSettingsToServer();
    void loadStudentList();
    void addStudent();
    void removeStudent();*/
    void onOpenSettings();
    void onSaveSettings();
    //void onAddStudents();
    void onAddStudents(const QStringList& names, QString group);

    void onStudentSelected();
    void onVariantDoubleClicked(int row, int column);
    //void sendStudentsToServer();
    void onEditStudentSettings();
    void shutdownServer();
    void onEditStudentVariant();
private:

    StudentSelectionWidget* studentSelectionWidget;
    QWidget*              studentObservationTab;

    QTcpSocket* socket;

    QHash<QString, studentInfo*> students;
    QHash<QString, QJsonArray> studentSettings;

    QStringList variants;

    QTableWidget* studentTable;
    QTextEdit* studentInfoTextEdit;
    SettingsDialog* settingsDialog;

    QJsonArray defaultSettings;

    QWidget* adminWidget;
    QWidget* settingsWidget;
    QWidget* monitorWidget;

    GroupsWidget* groupsTab;
    QWidget* variantsTab;
    QWidget* testsTab;

    QWidget* studentSelectionTab;





    void updateStudentRow(studentInfo* student);
    void setupStudentTable(QVBoxLayout* layout);
    QStringList readVariantsFromFile(const QString& filePath);
    void openVariantSelectionDialog(int row, int column);
    void updateStudentRow(const QJsonObject& studentsData);
    void sendStudentData(const QJsonObject& studentData);

    void sendUpdatedSettings(const QJsonArray& updatedSettings);


    void updateStudentsFromServer();
    void UpdateStudentsData(const QJsonArray& studentArray);
    void updateStudentTable(const QJsonArray& connectedStudents);
    void updateStudentTable();

    void setupUI();
    void createMenu();
    void setupConnections();

    void loadStudentResults(QString studentHash);
    //void loadStudentSettings(QString studentHash);
    void getDefaultSettings();
    void recDefaultSettings(const QJsonArray& response);
    void getStudentStructure();
    void loadStudentVariants(QString studentHash);

    void sendNewStudentListToServer(QVector<studentInfo*> newStudents);



    QJsonObject loadSettingsFromTXT(const QString& filePath);
    bool saveSettingsToTXT(const QJsonObject& settings);

    //QVector<studentInfo*> readStudentListFromCSV();

    void addStudent(studentInfo* newStudent);

    void sendUpdatedStudentsSettings(const QJsonArray& studentHashes, const QJsonArray& updatedSettings);

    void updateSelectedStudentFields(const QString& studentHash, const QStringList& selectedFieldIds);

    void onOpenGroups();
};
