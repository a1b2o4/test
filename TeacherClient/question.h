#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

enum class QuestionType {
    SingleChoice = 0,
    MultipleChoice,
    Match,
    Input
};

struct Question {
    int id = -1;
    QString text;
    QuestionType type = QuestionType::SingleChoice;
    int scoreCorrect = 0;
    int scoreWrong = 0;

    // Варианты ответов (если применимо)
    QStringList options;

    // Верные флаги (для одиночного/множественного выбора)
    QList<bool> correctFlags;

    // Верные ответы (для ввода и сопоставления)
    QStringList correctAnswers;

    // === Сериализация ===
    static QJsonObject toJson(const Question& q) {
        QJsonObject obj;
        obj["id"] = q.id;
        obj["text"] = q.text;
        obj["type"] = static_cast<int>(q.type);
        obj["scoreCorrect"] = q.scoreCorrect;
        obj["scoreWrong"] = q.scoreWrong;

        QJsonArray optArray;
        for (const QString& opt : q.options) optArray.append(opt);
        obj["options"] = optArray;

        if (q.type == QuestionType::SingleChoice || q.type == QuestionType::MultipleChoice) {
            QJsonArray flags;
            for (bool f : q.correctFlags) flags.append(f);
            obj["correctFlags"] = flags;
        } else {
            QJsonArray answers;
            for (const QString& a : q.correctAnswers) answers.append(a);
            obj["correctAnswers"] = answers;
        }

        return obj;
    }

    static Question fromJson(const QJsonObject& obj) {
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
};
