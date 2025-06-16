#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTcpSocket>
#include <QJsonArray>
#include <QJsonObject>

#include "diagramscene.h"

class TestWindow;

QT_BEGIN_NAMESPACE
class QToolBox;
class QSpinBox;
class QComboBox;
class QFontComboBox;
class QButtonGroup;
class QLineEdit;
class QGraphicsTextItem;
class QFont;
class QToolButton;
class QAbstractButton;
class QMenu;
class QAction;
class QGraphicsView;
QT_END_NAMESPACE

class StudentInfo;
class VariantInfo;
class VariantInfoDialog;
class ElementSelector;
class PropertiesTable;
class StudentInputDialog;
class NewGraphicsView;
class TestDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
//    MainWindow(StudentInfo* studentInfo, VariantInfo* variantInfo, QWidget *parent = nullptr);
    ~MainWindow();
    void connectToServer(const QString& host, quint16 port);

    static PropertiesTable* _propertiesTable;
    PropertiesTable* getPropertiesTable() { return propertiesTable; }
    void createActions();
    void createDockWidgets();
    void createMenu();

    QJsonArray getStudentsList() { return studentsList; }


    void setTestDialog(TestDialog* _testDialog) { testDialog = _testDialog; }
    TestDialog* getTestDialog() { return testDialog; }

    void updateSelectedStudentFields(const QStringList& selectedFieldIds);
    void setLaunchPass(bool Passed);

public slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

    void changeCurrTask(int prevTask, int selectedTask); // Сигнал, уведомляющий об изменении сцены
    void showTaskInfoDialog();

    void startScheme();

    void provideSetting(const QString &settingName);

    //void hidePropertiesPanel();
    //void showPropertiesPanel();


    void setEditMode();
    void setRunMode();
    void setLockSimMode();
    void deleteSelectedItem();

    void createVariants();

    void onTestFinished();
    void onUpdateTestResult(QMap<QString, QString>& results);

    void setEditPass(bool Passed);

    void updateStudentData();
    void showStudentSelectionDialog();

    void studentListCreated();

    void onBuildPath();
    void pathEnd();


signals:
    void studentsListLoaded();
    void sendSetting(const QString &settingName, const QVariant &value); // отправить настрой
    void setSchemeMode(SchemeMode _mode, bool changeMode = true);
    void studentListCreate();

private:
    QTcpSocket* socket;             // Сетевой сокет
    QJsonArray studentsList;
    bool studentsLoaded = false; // Флаг загрузки студентов
    QJsonObject selectedStudentInfo;

    QHash<QString, QPair<QString, QString>> studentSettings; // getSettingValue

    DiagramScene *scene;
    QGraphicsView *view;

    //StudentInputDialog* studentInputDialog;

    QAction *exitAction;

    //QToolBox *toolBox;
    ElementSelector* elementSelector;

    QMenu *fileMenu;
    QMenu *itemMenu;
    QMenu *aboutMenu;
    QMenu *testMenu;
    QAction* validateAction;
    QAction* simulateAction;
    QAction* startSchemeAction;
    QAction *editModeAction;
    QAction *runModeAction;
    QAction *lockSimModeAction;
    QAction* buildPathAction;

    StudentInfo* studentInfo;
    VariantInfo* variantInfo;
    PropertiesTable* propertiesTable;
    TestDialog* testDialog;

    void sendRequest(const QString& requestType, bool wait = true, const QJsonObject& data = QJsonObject());
    void handleResponse(const QByteArray& responseData);
    void studentSelected();
    void reqStudentSettings();  // Запрос настроек с сервера
    void reqGetStudentData();


    void hGetStudentList(const QJsonObject& responseData);
    void hGetStudentData(const QJsonObject& responseData);
    void hGetStudentSettings(const QJsonObject& settingsData);

    QVariant getSettingValue(const QString& settingName) const;
    void updateMenuState();  // функция обновления активности меню    StudentInfo* studentInfo;
    void updateModeAvailability(); // новое — обновление доступности переключателя режима

    QString buildPathPortNumber;
    QString buildPathControlPacket;


};
#endif // MAINWINDOW_H
