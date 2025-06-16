#ifndef PORT_H
#define PORT_H

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "connection.h"

class Port : public QObject
{

    Q_OBJECT

public:
    Port(SwitchingElement* switchingElement, QMap<QString, QString> properties);
    //Port(bool isInputPort, QString _numInElement, SwitchingElement* switchingElement, QString _numInStage = "0");
    ~Port();

    static int maxInputPortNum;
    static int maxOutputPortNum;

    // Геттеры и сеттеры
    QList<Connection*> getInputConnections() { return inputConnections; }
    QList<Connection*> getOutputConnections() { return outputConnections; }
    SwitchingElement* getSwitchingElement() { return switchingElement; }

    QString getNumInStage() const
    {
        return properties["Номер в ступени"];
    }
    QString getNumInElement() const { return properties["Номер в элементе"]; }
    QString getStage();

    void setInputConnection(Connection* connection) { inputConnections.append(connection); }
    void setOutputConnection(Connection* connection) { outputConnections.append(connection); }

    bool isInChannel() const { return inChannel; }
    bool isInputPort() const { return QVariant(properties["Вход"]).toBool(); }

    void setNumInElement(const QString _numInElement) { properties["Номер в элементе"] = _numInElement; }
    void setInChannel(bool _inChannel) { inChannel = _inChannel; }
    void setNumInStage(const QString _numInStage) { properties["Номер в ступени"] = _numInStage; }

    void removeInputConnection(Connection* conn);
    void removeOutputConnection(Connection* conn);


    void switchToNextOutput();

    QMap<QString, QString> getFields()
    {
        return properties;
    }

    void setFields(QMap<QString, QString> _properties)
    {
        properties["Номер в ступени"] = _properties.value("Номер в ступени", properties.value("Номер в ступени"," "));
        properties["Номер в элементе"] = _properties.value("Номер в элементе", properties.value("Номер в элементе"," "));
        properties["Вход"] = _properties.value("Вход", properties.value("Вход","0"));
    }

private:
    QList<Connection*> inputConnections;
    QList<Connection*> outputConnections;

    SwitchingElement* switchingElement;

    bool inChannel;

    QMap<QString, QString> properties; // inputPort numInElement numInStage
    //bool inputPort;
    //QString numInElement;          // номер в соответствующем массиве вх или вых портов switchingElement
    //QString numInStage = "";
};

#endif // PORT_H
