#include "testswidget.h"
#include "questiondialog.h"
#include <QToolButton>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

TestsWidget::TestsWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    loadFromFile("tests.json");
}

void TestsWidget::setupUi() {
    questionList = new QListWidget;
    testList = new QListWidget;
    descriptionEdit = new QTextEdit;
    descriptionEdit->setReadOnly(true);

    questionActions = new QToolButton;
    questionActions->setText("Действия   ");
    QMenu* qMenu = new QMenu(questionActions);
    qMenu->addAction("Добавить вопрос", this, &TestsWidget::onAddQuestion);
    qMenu->addAction("Редактировать вопрос", this, &TestsWidget::onEditQuestion);
    qMenu->addAction("Удалить вопрос", this, &TestsWidget::onRemoveQuestion);
    questionActions->setMenu(qMenu);
    questionActions->setPopupMode(QToolButton::InstantPopup);
    questionActions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    testActions = new QToolButton;
    testActions->setText("Действия   ");
    QMenu* tMenu = new QMenu(testActions);
    tMenu->addAction("Создать тест", this, &TestsWidget::onAddTest);
    tMenu->addAction("Удалить тест", this, &TestsWidget::onRemoveTest);
    testActions->setMenu(tMenu);
    testActions->setPopupMode(QToolButton::InstantPopup);
    testActions->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto* qLayout = new QVBoxLayout;
    auto* qMenuLayout = new QHBoxLayout;
    qMenuLayout->addWidget(new QLabel("Вопросы"));
    qMenuLayout->addWidget(questionActions);
    qLayout->addLayout(qMenuLayout);
    qLayout->addWidget(questionList);

    auto* tLayout = new QVBoxLayout;
    auto* tMenuLayout = new QHBoxLayout;
    tMenuLayout->addWidget(new QLabel("Тесты"));
    tMenuLayout->addWidget(testActions);
    tLayout->addLayout(tMenuLayout);
    tLayout->addWidget(testList);

    auto* dLayout = new QVBoxLayout;
    dLayout->addWidget(new QLabel("Описание"));
    dLayout->addWidget(descriptionEdit);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(qLayout, 2);
    mainLayout->addLayout(tLayout, 1);
    mainLayout->addLayout(dLayout, 3);

    connect(questionList, &QListWidget::currentRowChanged, this, &TestsWidget::onQuestionSelected);
    connect(testList, &QListWidget::currentRowChanged, this, &TestsWidget::onTestSelected);
}

void TestsWidget::onAddQuestion() {
    QuestionDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Question q = dlg.resultQuestion();
        q.id = nextQuestionId();
        questions.append(q);
        saveToFile("tests.json");
        refreshQuestionList();
    }
}

void TestsWidget::onEditQuestion() {
    int row = questionList->currentRow();
    if (row < 0 || row >= questions.size()) return;

    QuestionDialog dlg(this);
    dlg.setQuestion(questions[row]);
    if (dlg.exec() == QDialog::Accepted) {
        questions[row] = dlg.resultQuestion();
        questions[row].id = row;
        saveToFile("tests.json");
        refreshQuestionList();
    }
}

void TestsWidget::onRemoveQuestion() {
    int row = questionList->currentRow();
    if (row < 0 || row >= questions.size()) return;
    int id = questions[row].id;
    questions.remove(row);
    for (Test& t : tests) t.questionIds.removeAll(id);
    saveToFile("tests.json");
    refreshQuestionList();
    refreshTestList();
    descriptionEdit->clear();
}

void TestsWidget::onAddTest() {
    QList<int> selected;
    for (int i = 0; i < questionList->count(); ++i) {
        QListWidgetItem* item = questionList->item(i);
        if (item->checkState() == Qt::Checked)
            selected.append(questions[i].id);
    }
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не выбраны вопросы для теста.");
        return;
    }
    Test t;
    t.id = nextTestId();
    t.questionIds = selected;
    tests.append(t);
    saveToFile("tests.json");
    refreshTestList();
}

void TestsWidget::onRemoveTest() {
    int row = testList->currentRow();
    if (row < 0 || row >= tests.size()) return;
    tests.remove(row);
    saveToFile("tests.json");
    refreshTestList();
    descriptionEdit->clear();
}

void TestsWidget::onQuestionSelected() {
    int row = questionList->currentRow();
    if (row < 0 || row >= questions.size()) return;
    const Question& q = questions[row];
    QString desc = QString("Вопрос №%1\nТип: %2\n\n%3\n\nБаллы: +%4 / %5\n")
                       .arg(q.id)
                       .arg(typeToString(q.type))
                       .arg(q.text)
                       .arg(q.scoreCorrect)
                       .arg(q.scoreWrong);

    if (q.type == QuestionType::SingleChoice || q.type == QuestionType::MultipleChoice) {
        for (int i = 0; i < q.options.size(); ++i) {
            QString mark = q.correctFlags.value(i, false) ? "✔" : "✖";
            desc += QString("[%1] %2\n").arg(mark, q.options[i]);
        }
    } else if (q.type == QuestionType::Match) {
        for (const QString& a : q.correctAnswers)
            desc += "→ " + a + "\n";
    } else {
        for (const QString& a : q.correctAnswers)
            desc += a + "\n";
    }
    descriptionEdit->setText(desc);
}

void TestsWidget::onTestSelected() {
    int row = testList->currentRow();
    if (row < 0 || row >= tests.size()) return;

    const Test& t = tests[row];
    QString desc = QString("Тест №%1\n\n").arg(t.id);
    int i = 1;
    for (int qid : t.questionIds) {
        auto it = std::find_if(questions.begin(), questions.end(), [&](const Question& q) { return q.id == qid; });
        if (it != questions.end()) {
            desc += QString("%1. (%2) %3\nТип: %4\n")
                        .arg(i++)
                        .arg(it->id)
                        .arg(it->text)
                        .arg(typeToString(it->type));
            desc += QString("Баллы: +%1 / %2\n").arg(it->scoreCorrect).arg(it->scoreWrong);
            if (it->type == QuestionType::SingleChoice || it->type == QuestionType::MultipleChoice) {
                for (int j = 0; j < it->options.size(); ++j) {
                    QString mark = it->correctFlags.value(j, false) ? "✔" : "✖";
                    desc += QString("[%1] %2\n").arg(mark, it->options[j]);
                }
            } else if (it->type == QuestionType::Match) {
                for (const QString& a : it->correctAnswers)
                    desc += "→ " + a + "\n";
            } else {
                for (const QString& a : it->correctAnswers)
                    desc += a + "\n";
            }
            desc += "\n";
        }
    }
    descriptionEdit->setText(desc);
}

void TestsWidget::refreshQuestionList() {
    questionList->clear();
    for (int i = 0; i < questions.size(); ++i) {
        const Question& q = questions[i];
        QListWidgetItem* item = new QListWidgetItem(QString("Вопрос №%1: %2").arg(q.id).arg(q.text.left(30)));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        QColor bg = (i % 2 == 0) ? QColor("#ffffff") : QColor("#f0f0f0");
        item->setBackground(bg);
        questionList->addItem(item);
    }
}

void TestsWidget::refreshTestList() {
    testList->clear();
    for (const Test& t : tests) {
        testList->addItem(QString("Тест №%1").arg(t.id));
    }
}

QString TestsWidget::typeToString(QuestionType type) const {
    switch (type) {
    case QuestionType::SingleChoice: return "Одиночный выбор";
    case QuestionType::MultipleChoice: return "Множественный выбор";
    case QuestionType::Match: return "Сопоставление";
    case QuestionType::Input: return "Ввод слова";
    default: return "Неизвестно";
    }
}

void TestsWidget::saveToFile(const QString& path) {
    QJsonObject root;
    QJsonArray qarr;
    for (const Question& q : questions) {
        qarr.append(QuestionDialog::serializeQuestion(q));
    }
    root["questions"] = qarr;

    QJsonArray tarr;
    for (const Test& t : tests) {
        QJsonObject obj;
        obj["id"] = t.id;
        QJsonArray ids;
        for (int id : t.questionIds) ids.append(id);
        obj["questionIds"] = ids;
        tarr.append(obj);
    }
    root["tests"] = tarr;

    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson());
        f.close();
    }
}

void TestsWidget::loadFromFile(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    questions.clear();
    tests.clear();

    QJsonObject root = doc.object();
    for (const QJsonValue& v : root["questions"].toArray())
        questions.append(QuestionDialog::deserializeQuestion(v.toObject()));

    for (const QJsonValue& v : root["tests"].toArray()) {
        QJsonObject o = v.toObject();
        Test t;
        t.id = o["id"].toInt();
        for (const QJsonValue& id : o["questionIds"].toArray())
            t.questionIds.append(id.toInt());
        tests.append(t);
    }

    refreshQuestionList();
    refreshTestList();
}

int TestsWidget::nextQuestionId() const {
    int max = 0;
    for (const Question& q : questions) max = std::max(max, q.id);
    return max + 1;
}

int TestsWidget::nextTestId() const {
    int max = 0;
    for (const Test& t : tests) max = std::max(max, t.id);
    return max + 1;
}
