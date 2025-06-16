#pragma once

#include <QDialog>
#include <QList>
#include <QJsonObject>
#include "question.h"

class QComboBox;
class QTextEdit;
class QSpinBox;
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QDialogButtonBox;
class QVBoxLayout;
class QFormLayout;
class QLineEdit;

class QuestionDialog : public QDialog {
    Q_OBJECT

public:
    explicit QuestionDialog(QWidget* parent = nullptr);

    Question resultQuestion() const;
    void setQuestion(const Question& q);

    static QJsonObject serializeQuestion(const Question& q);
    static Question deserializeQuestion(const QJsonObject& obj);
    static bool saveQuestionsToFile(const QString& path, const QList<Question>& list);
    static QList<Question> loadQuestionsFromFile(const QString& path);

private slots:
    void onTypeChanged(const QString&);
    void onAddOption();
    //void onOptionChanged(QListWidgetItem*);

private:
    void setupUi();
    void clearAnswerLayout();

    QComboBox* typeBox = nullptr;
    QTextEdit* questionTextEdit = nullptr;
    QSpinBox* correctPoints = nullptr;
    QSpinBox* wrongPoints = nullptr;

    QWidget* answerWidget = nullptr;
    QVBoxLayout* answerLayout = nullptr;

    QListWidget* answerList = nullptr;  // Для одиночного и множественного выбора
    QListWidget* inputList = nullptr;   // Для ввода слова
    QFormLayout* matchLayout = nullptr; // Для сопоставления

    QList<QLineEdit*> matchLeft;
    QList<QLineEdit*> matchRight;

    QPushButton* addOptionButton = nullptr;
    QDialogButtonBox* buttonBox = nullptr;
};
