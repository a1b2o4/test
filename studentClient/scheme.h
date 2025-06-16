#ifndef SCHEME_H
#define SCHEME_H

#include "connection.h"
#include "port.h"
#include "switchingelement.h"
#include "topology.h"
#include "diagramscene.h"

#include <QString>
#include <QVector>
#include <QList>
#include <QCryptographicHash>

class Scheme : public QObject
{
    Q_OBJECT

public:
    Scheme(Topology* topology);

    SwitchingElement* addSwitchingElement(QMap<QString, QString> properties);
    //Port* addIOPort(bool inputPort);
    Port* addIOPort(QMap<QString, QString> _properties);
    Connection* addConnection(Port *startItem, Port *endItem, bool inConnection = false);

    QString createCurrSchemeString();
    QList<QString> checkScheme();


    Topology* getTopology() { return topology; }

    void getTokenPath(Port* startPort, QString& controlPacket, bool* isBlock);
    Port* findPortByNumber(QString portNumber);

    QSet<Port*> getOccupiedPorts() { return occupiedPorts; }
    QList<Connection*> getPathConnections() { return pathConnections; }


    void removeSwitchingElement(SwitchingElement* switchingElement);
    void removePort(Port* port);

    static QString generateHash(QString str)
    {
        return QString(QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5).toHex());
    }

    QList<Port*> getPathPorts() { return pathPorts; }


public slots:
    void changedSchemeMode(SchemeMode _mode);
    void removeConnection(Connection* connection);

private:
    QStringList splitWithDelimiters(const QString& str, const QString& delimiters);

    void processStage(Port*& currentPort, QString controlPacket, bool* isBlock);

    Connection* getNextConnection(Port* port);
    Port* getNextPort(Connection* conn, bool* isBlock);
    Port* getOutPortInElement(Port* currentPort, QString controlPacket, bool* isBlock);

    QList<Port*> inputPorts;
    QList<Port*> outputPorts;
    QList<SwitchingElement*> swEl;
    QList<QPointer<Connection>> connections;

    Topology* topology;

    int portBase;


    QSet<Port*> occupiedPorts;
    QList<Connection*> pathConnections;
    QList<Port*> pathPorts;


};

#endif // SCHEME_H
