#ifndef CLOSA_H
#define CLOSA_H

#include "topology.h"

class Closa : public Topology
{
public:
    Closa(int _inputCrossCount, int _outputCrossCount, int _midCrossCount, int _inputInCrossInInput, int _outputInCrossInOutput);

    void setStagePriorities(int stageNum, const QMap<int, int>& updatedPriorities)
    {
        if (stageNum < 0 || stageNum >= stagePriorities.size()) return;

        for (auto it = updatedPriorities.begin(); it != updatedPriorities.end(); ++it)
        {
            stagePriorities[stageNum][it.key()] = it.value();
        }
    }

    QMap<int, int> getStagePriorities(int stageNum) const
    {
        return stagePriorities.contains(stageNum) ? stagePriorities[stageNum] : QMap<int, int>();
    }
protected:
    QString getDescription() const override;
    QString getNextPortNum(const QString portNum, int stageNum, int base = 2) const override;
    Port* getOutPortInElement(Port* currentPort, QString controlPacket, QSet<Port*> occupiedPorts) override;

    QString createReferenceString() const override;
    bool checkControlPacket(QString controlPacket) override;

private:
    int inputCrossCount;    // r1 к-во кроссбаров = к-ву входов в кроссбаре средней ступени
    int outputCrossCount;   // r2
    int midCrossCount;      // m к-во кроссбаров = к-ву входов в кроссбаре последней ступени
    int inputInCrossInInput;//n1
    int outputInCrossInOutput;//n2

    QList<int> countInputInCrossInStage;
    QList<int> countOutputInCrossInStage;


    QMap<int, QMap<int, int>> stagePriorities;  // ступень, номер выхода = приоритет
};

#endif // CLOSA_H
