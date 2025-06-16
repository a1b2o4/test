#include "banyan.h"
#include "port.h"
#include "switchingelement.h"


#include <cmath>

QString Banyan::createReferenceString() const
{
    QString rstr = "";
    QString rstrC = ""; // для строки соединений
    QString nextPortNum;

    int n = getInputCount();
    int m = getOutputCount();
    int s = getStageCount();
    //int k = getCountInStage();
    int nLength = log2(n);
    int mLength = log2(m);

    qDebug()<<n << "n";

    QString secondStageNum;

    int eNCount = 2;
    int eMCount = 2;

    for (int inputNum = 0; inputNum < n; inputNum++)
    {
        nextPortNum = Topology::perfectShuffle(Topology::intToBase(inputNum, 2, nLength)); //тасование входов
        rstr.append("I" + Topology::intToBase(inputNum, 2, nLength)); // номер входа сети

        rstrC.append("C*");
        rstrC.append(" *");                                             // нет эл-та, поэтому - пробел
        rstrC.append(Topology::intToBase(inputNum, 2, nLength) + "*");  // первая точка соединения
        //rstrC.append(QString::number(int(inputNum/2)) + "*");           // номер эл-та второй точки в след ступени
        rstrC.append("0*");           // номер след ступени
        rstrC.append(nextPortNum);                                      // вторая точка соединения
    }

    rstr.append("|"); // отделяет входы и элементы

    //rstr.append("|"); // отделяет входы и элементы
    //rstr.append("|"); // отделяет элементы и входы
    //rstr.append("|"); // отделяет входы и соединения

    for (int stageNum = 0; stageNum < s; stageNum++)
    {        
        for (int elementNum = 0; elementNum < getCountInStage(stageNum); elementNum++)
        {
            rstr.append("S2-2");    //количество входов и выходов
            rstr.append("L" + QString::number(stageNum));   // номер ступени
            rstr.append("N" + QString::number(elementNum)); // номер элемента в ступени

            for (int eInputNum = 0; eInputNum < eNCount; eInputNum++)
            {
                rstr.append("i" + QString::number(eInputNum) + "*");  // номер входа в элементе
                rstr.append(Topology::intToBase(elementNum * eNCount + eInputNum, 2, nLength) + "*");   // номер входа в ступени
                rstr.append(QString::number(stageNum) + "*");   // ступень
                rstr.append(QString::number(elementNum));       // номер элемента для тек входа в ступени
            }

            for (int eOutputNum = 0; eOutputNum < eNCount; eOutputNum++)
            {
                rstr.append("o" + QString::number(eOutputNum) + "*");  // номер выхода в элементе
                rstr.append(Topology::intToBase(elementNum * eMCount + eOutputNum, 2, nLength) + "*");   // номер выхода в ступени
                rstr.append(QString::number(stageNum) + "*");  // ступень
                rstr.append(QString::number(elementNum));      // номер элемента для тек выхода в ступени

                if (stageNum < (s - 1))
                {
                    secondStageNum = QString::number(stageNum + 1);
                    nextPortNum = getNextPortNum(Topology::intToBase(elementNum * eMCount + eOutputNum, 2, nLength), stageNum);
                }
                else
                {
                    secondStageNum = " ";
                    nextPortNum = Topology::intToBase(elementNum * eMCount + eOutputNum, 2, nLength);
                }

                rstrC.append("C*");
                rstrC.append(QString::number(stageNum) + "*");  // ступень
                rstrC.append(Topology::intToBase(elementNum * eMCount + eOutputNum, 2, nLength) + "*"); // первая точка соединения
                rstrC.append(secondStageNum + "*");             // номер ступени второй точки
                rstrC.append(nextPortNum);                      // вторая точка соединения (если не последняя ступень, то высчитать номер, иначе такой же
            }
        }
    }

    rstr.append("|"); // отделяет элементы и входы

    for (int outputNum = 0; outputNum < m; outputNum++)
    {
        rstr.append("O" + Topology::intToBase(outputNum, 2, mLength));  // номер выхода сети
    }

    rstr.append("|"); // отделяет входы и соединения

    rstr.append(rstrC);
    qDebug()<< "banyan" << " " << rstr;
    return rstr;
}

QString Banyan::getNextPortNum(const QString portNum, int stageNum, int bas) const
{
    QString tmpPortNum = portNum;
    int tstage = stageNum;
    int firstIndex = tmpPortNum.length() - 1;

    QChar tmpChar = tmpPortNum[tstage];

    tmpPortNum[tstage] = tmpPortNum[firstIndex];
    tmpPortNum[firstIndex] = tmpChar;

    return tmpPortNum;
}

QString Banyan::getDescription() const
{
    QString description = QString("%1. %2\n").arg("Топология: ").arg("Баньян");

    description += QString("Количество входов: %1\n").arg(getInputCount());
    description += QString("Количество выходов: %1\n").arg(getOutputCount());

    qDebug()<<"topology "<<description;

    return description;
}

Port* Banyan::getOutPortInElement(Port* currentPort, QString controlPacket, QSet<Port*> occupiedPorts)
{

    SwitchingElement* SwEl = currentPort->getSwitchingElement();
    qDebug() << "SwEl " << !SwEl;
    SwEl->setControlPacket(controlPacket);
    qDebug() << "setControlPacket " << controlPacket;

    //------ получение outputIndex
    int outputIndex = controlPacket[SwEl->getStage().toInt()].digitValue();
    //------ получение outputIndex

    qDebug() << "outputIndex " << outputIndex;
    currentPort = SwEl->getOutputPorts()[outputIndex];
    qDebug() << "currentPort " << !currentPort;

    return currentPort;
}

Banyan::Banyan(int inputCount)//, int stageCount, int selfRouting)
    :Topology(inputCount, inputCount, log2(inputCount), 1, 2)
{
    QList<int> countInStage;
    for(int i = 0; i < log2(inputCount); i++)
    {
        countInStage.append(inputCount / 2);
    }

    setCountInStage(countInStage);
    referenceString = createReferenceString();
}

bool Banyan::checkControlPacket(QString controlPacket)
{
    if (controlPacket.length() != getStageCount())
        return false;

    for (QChar ch : controlPacket)
    {
        if (!ch.isDigit())
            return false;

        int value = ch.digitValue();
        if ((value < 0) || (value >= 2))
            return false;
    }

    return true;
}
