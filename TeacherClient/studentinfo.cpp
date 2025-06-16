#include "studentinfo.h"

#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QJsonArray studentInfo::studentFields;

studentInfo::studentInfo(const QJsonObject& fields)
{
    int totalFields = studentInfo::studentFields.size();  // Получаем количество полей из studentFields

    // Создаем копию входного объекта данных
    QJsonObject updatedFields = fields;

    // Дополняем поле "fields", если каких-то данных не хватает
    for (int i = 0; i < totalFields; ++i)
    {
        QJsonObject field = studentInfo::studentFields.at(i).toObject();  // Используем at() для получения элемента

        QString fieldId = field["id"].toString();
        if (!updatedFields.contains(fieldId))
        {
            updatedFields.insert(fieldId, field["defaultValue"].toString());  // Добавляем значение по умолчанию, если его нет
        }
    }

    // Передаем обновленные поля в метод установки полей
    setFields(updatedFields);
}

void studentInfo::setFields(const QJsonObject& fields)
{
    for (const QJsonValue& value : studentInfo::studentFields)     // Перебираем элементы в QJsonArray
    {
        if (value.isObject())  // Проверяем, что это действительно QJsonObject
        {
            QJsonObject field = value.toObject();  // Извлекаем QJsonObject

            QString fieldID = field["id"].toString();
            QString defaultValue = field["defaultValue"].toString();    // Значение по умолчанию из структуры

            if (fields.contains(fieldID))  // Проверяем, есть ли значение для данного поля в данных
            {
                QString fieldValue = fields[fieldID].toString();
                studentsData[fieldID] = fieldValue.isEmpty() ? defaultValue : fieldValue; // Если значение пустое, используем значение по умолчанию
            }
            else
            {
                studentsData[fieldID] = defaultValue.isEmpty() ? "-" : defaultValue; // Если поле не присутствует в данных, устанавливаем значение по умолчанию
            }
        }
    }

    studentsData["hash"] = generateHash();  // Генерация хэша и добавление в studentsData
}

QString studentInfo::getStudentInfo()
{
    QString studentInfoStr = "";

    for (const QJsonValue& val : studentFields)
    {
        if (!val.isObject()) continue;

        QJsonObject fieldObj = val.toObject();
        if (!fieldObj["showInTable"].toBool()) continue;

        QString fieldId = fieldObj["id"].toString();
        QString label = fieldObj["label"].toString();

        QString value = studentsData.value(fieldId).toString();
        studentInfoStr += label + ": " + value + "\n";
    }
    return studentInfoStr;
}

void studentInfo::loadStudentFields(const QJsonObject& data)
{
    if (!data.contains("structure") || !data["structure"].isArray()) {
        qDebug() << "Ошибка: данные структуры студентов отсутствуют или имеют некорректный формат!";
        return;
    }

    QJsonArray jsonArray = data["structure"].toArray();

    studentFields = jsonArray;    // Очищаем текущую структуру, если необходимо

    qDebug() << "Структура студентов загружена:" << studentFields;
}

void studentInfo::loadStudentFields()
{
 /*   QFile file("students_structure.json");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Ошибка открытия структуры данных!";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray())
    {
        qDebug() << "Некорректный формат структуры!";
        return;
    }

    QJsonArray jsonArray = doc.array();
    studentFields.clear();

    for (const QJsonValue& value : jsonArray)
    {
        if (value.isObject())
        {
            QJsonObject obj = value.toObject();
            studentFields.append(obj);  // добавляем в список
        }
    }

    qDebug() << "Структура загружена: " << studentFields;*/
}

QVector<studentInfo*> studentInfo::readStudentListFromCSV()
{
    QVector<studentInfo*> newStudents;

    // Открываем диалоговое окно выбора файла
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,  // Родительский виджет
        "Выберите файл",  // Заголовок окна
        "",  // Начальный путь (пусто, чтобы использовать стандартный)
        "CSV Files (*.csv);;All Files (*)"  // Фильтры для выбора файлов
        );
    qDebug() << "1: ";

    // Проверка, если пользователь отменил выбор или не выбрал файл
    if (fileName.isEmpty())
    {
        qDebug() << "Файл не выбран.";
        return {};  // Возвращаем пустой список студентов
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Не удалось открыть файл:" << fileName;
        return {};  // Возвращаем пустой список, если файл не открылся
    }

    QTextStream in(&file);

    // Читаем первую строку с идентификаторами полей (эти поля присутствуют в файле)
    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(",");

    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.isEmpty() || line.startsWith("//"))
            continue;

        // Разбиваем строку на части
        QStringList fields = line.split(",");

        // Создаем QJsonObject для данных студента
        QJsonObject studentData;
        for (int i = 0; i < headers.size(); i++)
        {
            studentData.insert(headers[i], fields[i]);  // Заполняем QJsonObject
        }

        studentInfo* newStudent = new studentInfo(studentData);  // Передаем QJsonObject в конструктор
        newStudents.append(newStudent);
    }

    file.close();
    return newStudents;
}


