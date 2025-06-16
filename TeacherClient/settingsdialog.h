#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const QJsonArray& settings, QWidget* parent = nullptr);
    //~SettingsDialog();

    QJsonArray getUpdatedSettings() const;
    void updateSettings(const QJsonArray& newSettings);

private:
    QJsonArray settings;

    QTableWidget* tableWidget;

    QListWidget* listWidget;
    QLabel* nameLabel;
    QLineEdit* valueEdit;
    QPushButton* saveButton;

private slots:
    //void onSettingSelected(QListWidgetItem* current);
    void onSaveClicked();
};

#endif // SETTINGSDIALOG_H
