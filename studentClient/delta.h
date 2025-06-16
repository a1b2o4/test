#ifndef DELTA_H
#define DELTA_H

#include "topology.h"

class Delta : public Topology
{
public:
    Delta(int _stage, int _inputCountCross, int _outputCountCross);

protected:
    QString getDescription() const override;
    QString getNextPortNum(const QString portNum, int stageNum, int base = 2) const override;
    Port* getOutPortInElement(Port* currentPort, QString controlPacket, QSet<Port*> occupiedPorts) override;

    QString createReferenceString() const override;
    bool checkControlPacket(QString controlPacket) override;

    int getInputCountCross() const { return inputCountCross; }
    int getOutputCountCross() const { return outputCountCross; }

private:
    int inputCountCross; // количество входов коммутаторов (степень счисления входов сети)
    int outputCountCross;
};

#endif // DELTA_H
