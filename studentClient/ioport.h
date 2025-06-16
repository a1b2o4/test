/*#ifndef IOPORT_H
#define IOPORT_H

#include "port.h"
#include <QObject>

class IOPort : QObject
{

    Q_OBJECT

public:
    IOPort(bool inputPort, QString numInStage = "000", int _numInElement = 0);

    //void setPortNum(int portNum);

    //void setInputPort(bool input);

    //QString getBinaryPortNum(int base = 2) const;

    Port* getPort() { return port; }
    int getNumInElement() const { return port->getNumInElement(); }
    QString getNumInStage() const { return port->getNumInStage(); }
    bool isInputPort() const { return inputPort; }

    QList<QPair<QString, QVariant>> getFields()
    {
        QList<QPair<QString, QVariant>> fields;
        fields.append( {"Номер в ступени", port->getNumInStage()} );

        return fields;
    }

    void setFields(QList<QString> properties) { port->setNumInStage(properties[0]); }

signals:
    void portModified();

private:
    Port* port;
    bool inputPort;
};

#endif // IOPORT_H
*/
