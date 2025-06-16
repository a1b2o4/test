#include "questiondialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QTextEdit>
#include <QSpinBox>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QuestionDialog::QuestionDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Вопрос");
    resize(500, 500);
    setupUi();
}

void QuestionDialog::setupUi() {
    auto* layout = new QVBoxLayout(this);

    typeBox = new QComboBox(this);
    typeBox->addItems({"Одиночный выбор", "Множественный выбор", "Сопоставление", "Ввод слова"});
    connect(typeBox, &QComboBox::currentTextChanged, this, &QuestionDialog::onTypeChanged);

    questionTextEdit = new QTextEdit(this);

    correctPoints = new QSpinBox(this);
    correctPoints->setRange(-100, 100);
    correctPoints->setValue(10);

    wrongPoints = new QSpinBox(this);
    wrongPoints->setRange(-100, 100);
    wrongPoints->setValue(0);

    QWidget* answerWidget = new QWidget(this);
    answerLayout = new QVBoxLayout(answerWidget);

    addOptionButton = new QPushButton("Добавить", this);
    connect(addOptionButton, &QPushButton::clicked, this, &QuestionDialog::onAddOption);

    auto* form = new QFormLayout;
    form->addRow("Тип вопроса:", typeBox);
    form->addRow("Текст вопроса:", questionTextEdit);
    form->addRow("Баллы за правильный ответ:", correctPoints);
    form->addRow("Баллы за неправильный ответ:", wrongPoints);

    layout->addLayout(form);
    layout->addWidget(new QLabel("Ответы:"));
    layout->addWidget(answerWidget);
    layout->addWidget(addOptionButton);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    onTypeChanged(typeBox->currentText());
}

void QuestionDialog::clearAnswerLayout() {
    QLayoutItem* child;
    while ((child = answerLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    matchLeft.clear();
    matchRight.clear();
}

void QuestionDialog::onTypeChanged(const QString& type) {
    clearAnswerLayout();

    if (type == "Одиночный выбор" || type == "Множественный выбор") {
        answerList = new QListWidget(this);
        answerLayout->addWidget(answerList);
    } else if (type == "Сопоставление") {
        matchLayout = new QFormLayout;
        answerLayout->addLayout(matchLayout);
    } else if (type == "Ввод слова") {
        inputList = new QListWidget(this);
        answerLayout->addWidget(inputList);
    }
}

void QuestionDialog::onAddOption() {
    QString type = typeBox->currentText();
    if (type == "Одиночный выбор" || type == "Множественный выбор") {
        QListWidgetItem* item = new QListWidgetItem("", answerList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setCheckState(Qt::Unchecked);
        answerList->addItem(item);
        answerList->editItem(item);
    } else if (type == "Сопоставление") {
        QLineEdit* left = new QLineEdit;
        QLineEdit* right = new QLineEdit;
        matchLayout->addRow(left, right);
        matchLeft.append(left);
        matchRight.append(right);
    } else if (type == "Ввод слова") {
        QListWidgetItem* item = new QListWidgetItem("", inputList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        inputList->addItem(item);
        inputList->editItem(item);
    }
}

Question QuestionDialog::resultQuestion() const {
    Question q;
    q.id = -1;
    q.text = questionTextEdit->toPlainText();
    q.scoreCorrect = correctPoints->value();
    q.scoreWrong = wrongPoints->value();
    q.type = static_cast<QuestionType>(typeBox->currentIndex());

    if (q.type == QuestionType::SingleChoice || q.type == QuestionType::MultipleChoice) {
        for (int i = 0; i < answerList->count(); ++i) {
            QListWidgetItem* item = answerList->item(i);
            q.options.append(item->text());
            q.correctFlags.append(item->checkState() == Qt::Checked);
        }
    } else if (q.type == QuestionType::Input) {
        for (int i = 0; i < inputList->count(); ++i) {
            QListWidgetItem* item = inputList->item(i);
            q.correctAnswers.append(item->text());
        }
    } else if (q.type == QuestionType::Match) {
        for (int i = 0; i < matchLeft.size(); ++i) {
            q.options.append(matchLeft[i]->text());
            q.correctAnswers.append(matchRight[i]->text());
        }
    }

    return q;
}

void QuestionDialog::setQuestion(const Question& q) {
    typeBox->setCurrentIndex(static_cast<int>(q.type));
    questionTextEdit->setPlainText(q.text);
    correctPoints->setValue(q.scoreCorrect);
    wrongPoints->setValue(q.scoreWrong);

    onTypeChanged(typeBox->currentText());

    if (q.type == QuestionType::SingleChoice || q.type == QuestionType::MultipleChoice) {
        for (int i = 0; i < q.options.size(); ++i) {
            QListWidgetItem* item = new QListWidgetItem(q.options[i], answerList);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            item->setCheckState(q.correctFlags.value(i) ? Qt::Checked : Qt::Unchecked);
        }
    } else if (q.type == QuestionType::Input) {
        for (const QString& a : q.correctAnswers) {
            QListWidgetItem* item = new QListWidgetItem(a, inputList);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    } else if (q.type == QuestionType::Match) {
        for (int i = 0; i < q.options.size(); ++i) {
            QLineEdit* left = new QLineEdit(q.options[i]);
            QLineEdit* right = new QLineEdit(q.correctAnswers.value(i));
            matchLayout->addRow(left, right);
            matchLeft.append(left);
            matchRight.append(right);
        }
    }
}

QJsonObject QuestionDialog::serializeQuestion(const Question& q) {
    QJsonObject obj;
    obj["id"] = q.id;
    obj["text"] = q.text;
    obj["type"] = static_cast<int>(q.type);
    obj["scoreCorrect"] = q.scoreCorrect;
    obj["scoreWrong"] = q.scoreWrong;

    QJsonArray opts;
    for (const QString& o : q.options) opts.append(o);
    obj["options"] = opts;

    if (q.type == QuestionType::SingleChoice || q.type == QuestionType::MultipleChoice) {
        QJsonArray flags;
        for (bool b : q.correctFlags) flags.append(b);
        obj["correctFlags"] = flags;
    } else {
        QJsonArray answers;
        for (const QString& a : q.correctAnswers) answers.append(a);
        obj["correctAnswers"] = answers;
    }

    return obj;
}

Question QuestionDialog::deserializeQuestion(const QJsonObject& obj) {
    Question q;
    q.id = obj["id"].toInt();
    q.text = obj["text"].toString();
    q.type = static_cast<QuestionType>(obj["type"].toInt());
    q.scoreCorrect = obj["scoreCorrect"].toInt();
    q.scoreWrong = obj["scoreWrong"].toInt();

    for (const QJsonValue& v : obj["options"].toArray())
        q.options << v.toString();

    if (q.type == QuestionType::SingleChoice || q.type == QuestionType::MultipleChoice) {
        for (const QJsonValue& v : obj["correctFlags"].toArray())
            q.correctFlags << v.toBool();
    } else {
        for (const QJsonValue& v : obj["correctAnswers"].toArray())
            q.correctAnswers << v.toString();
    }

    return q;
}

bool QuestionDialog::saveQuestionsToFile(const QString& path, const QList<Question>& list) {
    QJsonArray arr;
    for (const Question& q : list)
        arr.append(serializeQuestion(q));

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(arr).toJson());
    f.close();
    return true;
}

QList<Question> QuestionDialog::loadQuestionsFromFile(const QString& path) {
    QList<Question> list;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return list;

    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return list;

    for (const QJsonValue& v : doc.array())
        list << deserializeQuestion(v.toObject());

    return list;
}
