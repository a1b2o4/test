// testswidget.h
#pragma once

#include <QWidget>
#include <QList>
#include "question.h"

class QListWidget;
class QTextEdit;
class QToolButton;

struct Test {
    int id;
    QList<int> questionIds;
};

class TestsWidget : public QWidget {
    Q_OBJECT

public:
    explicit TestsWidget(QWidget* parent = nullptr);

private slots:
    void onAddQuestion();
    void onEditQuestion();
    void onRemoveQuestion();
    void onAddTest();
    void onRemoveTest();
    void onQuestionSelected();
    void onTestSelected();

private:
    void setupUi();
    void refreshQuestionList();
    void refreshTestList();
    void updateDescription();
    QString typeToString(QuestionType type) const;


    int nextQuestionId() const;
    int nextTestId() const;
    void saveToFile(const QString& path);
    void loadFromFile(const QString& path);

    QListWidget* questionList = nullptr;
    QListWidget* testList = nullptr;
    QTextEdit* descriptionEdit = nullptr;
    QToolButton* questionActions = nullptr;
    QToolButton* testActions = nullptr;

    QList<Question> questions;
    QList<Test> tests;
};
