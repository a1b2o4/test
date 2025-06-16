#ifndef STUDENTINFO_H
#define STUDENTINFO_H

#include <QObject>
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonObject>

class QString;

class studentInfo : public QObject
{
    Q_OBJECT

public:
    studentInfo(const QJsonObject& fields);

    static QJsonArray studentFields;    // важен порядок

    static QVector<studentInfo*> readStudentListFromCSV();
    static void loadStudentFields();
    static void loadStudentFields(const QJsonObject& data);

    bool getConnected() const { return studentsData.contains("connected") && studentsData["connected"] == "true";    }
    void setConnected(bool _connected) { studentsData["connected"] = _connected ? "true" : "false"; }


    QString getHash() const { return studentsData["hash"].toString(); }
    QJsonObject getStudentsData() { return studentsData; }
    QString getStudentInfo();

    void setFields(const QJsonObject& fields);
    void setStudentsData(QJsonObject updatedData) { studentsData= updatedData; }
    void setVariant(QString newVariant) { studentsData["variant"] = newVariant; }  // Обновляем в studentInfo


    QString generateHash() const
    {
        QString data = QString("%1|%2")
                           .arg(studentsData["name"].toString())
                           .arg(studentsData["group"].toString());
        return QString(QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Md5).toHex());
    }

private:
    QJsonObject studentsData;   // не важен порядок
};

#endif // STUDENTINFO_H
