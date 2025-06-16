#ifndef BANYAN_H
#define BANYAN_H

#include "topology.h"

class Banyan: public Topology
{
public:
    Banyan(int inputNum);

protected:
    QString getDescription() const override;
    QString getNextPortNum(const QString portNum, int stageNum, int base = 2) const override;
    Port* getOutPortInElement(Port* currentPort, QString controlPacket, QSet<Port*> occupiedPorts) override;


    QString createReferenceString() const override;
    bool checkControlPacket(QString controlPacket) override;

};

#endif // BANYAN_H
