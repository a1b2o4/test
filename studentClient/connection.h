#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QList>

//#include "port.h"

class Port;
class SwitchingElement;

class Connection : public QObject
{
    Q_OBJECT

public:
    Connection(Port *startItem, Port *endItem, bool _innerConnection = false);
    ~Connection();


    Port* getStartItem() const { return myStartItem; }


    void setStartItem(Port *startItem) { myStartItem = startItem; }
    void setEndItem(Port *endItem) { myEndItem = endItem; }

    void setInChannel(bool inChannel) { this->inChannel = inChannel; }
    bool getInChannel() const { return inChannel; }

    void setInner(bool inner) { innerConnection = inner; }
    bool isInner() { return innerConnection; }

    void changeEndItem(Port* currentPort);
    Port* getEndItem() const;

    //QList<QPointF> getLinePoints() const;

signals:
    void endItemChanged(Port* newPort);
    void deleteQConnection();
    void isDeleted(Connection* conn);

protected:
    Port *myStartItem;
    Port *myEndItem;

    bool innerConnection;

    bool inChannel;
};

#endif // CONNECTION_H
