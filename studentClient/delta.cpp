#include "delta.h"
#include "port.h"
#include "switchingelement.h"
QString Delta::createReferenceString() const
{
    QString rstr = "";
    QString rstrC = ""; // для строки соединений
    QString nextPortNum;
    QString portNum;

    int n = getInputCount();    // количество входов сети
    int m = getOutputCount();   // количество выходов сети
    int s = getStageCount();    // количество стадий
    //int k = getCountInStage();
    //int base = getInputCountCross();
    //int base = getOutputCountCross();
    int base = 2;
    int nLength = std::ceil(log10(fmax(n,m)) / log10(base));

    qDebug()<< nLength;         // длина входов/выходов
    qDebug()<<"fmax "<< fmax(n,m);
    qDebug()<<"n "<< n;
    qDebug()<<"m "<< m;
    qDebug()<<"s "<< s;
    qDebug()<<"base "<< base;

    //qDebug()<< mLength;

    QString secondStageNum;

    int eNCount = getInputCountCross();
    int eMCount = getOutputCountCross();

    qDebug()<<"eNCount "<< eNCount;
    qDebug()<<"eMCount "<< eMCount;
    qDebug()<<"n "<< n;

    for (int inputNum = 0; inputNum < n; inputNum++)
    {
        qDebug()<<"inputNum "<< inputNum;
        portNum = Topology::intToBase(inputNum, base, nLength);
        nextPortNum = Topology::intToBase(inputNum, base, nLength);    // отсутствие тасование входов

        rstr.append("I" + portNum);     // номер входа сети

        rstrC.append("C*");
        rstrC.append(" *");             // нет эл-та, поэтому - пробел
        rstrC.append(portNum + "*");    // первая точка соединения
        //rstrC.append(QString::number(int(inputNum/2)) + "*");         // номер эл-та второй точки в след ступени
        rstrC.append("0*");             // номер след ступени
        rstrC.append(nextPortNum);      // вторая точка соединения
    }
    rstr.append("|"); // отделяет входы и элементы

    qDebug()<<"s "<< s;
    for (int stageNum = 0; stageNum < s; stageNum++)
    {
        qDebug()<<"stageNum "<< stageNum;
        qDebug()<<"getCountInStage(stageNum) "<< getCountInStage(stageNum);
        for (int elementNum = 0; elementNum < getCountInStage(stageNum); elementNum++)
        {
            qDebug()<<"elementNum "<< elementNum;
            //rstr.append("S2-2");    //количество входов и выходов
            rstr.append("S" + QString::number(eNCount) + "-" + QString::number(eMCount));    //количество входов и выходов
            rstr.append("L" + QString::number(stageNum));   // номер ступени
            rstr.append("N" + QString::number(elementNum)); // номер элемента в ступени

            qDebug()<<"eNCount "<< eNCount;
            for (int eInputNum = 0; eInputNum < eNCount; eInputNum++)
            {
                qDebug()<<"eInputNum "<< eInputNum;
                rstr.append("i" + QString::number(eInputNum) + "*");  // номер входа в элементе
                rstr.append(Topology::intToBase(elementNum * eNCount + eInputNum, base, nLength) + "*");   // номер входа в ступени
                rstr.append(QString::number(stageNum) + "*");   // ступень
                rstr.append(QString::number(elementNum));       // номер элемента для тек входа в ступени
            }

            for (int eOutputNum = 0; eOutputNum < eMCount; eOutputNum++)
            {
                rstr.append("o" + QString::number(eOutputNum) + "*");  // номер выхода в элементе
                rstr.append(Topology::intToBase(elementNum * eMCount + eOutputNum, base, nLength) + "*");   // номер выхода в ступени
                rstr.append(QString::number(stageNum) + "*");  // ступень
                rstr.append(QString::number(elementNum));      // номер элемента для тек выхода в ступени

                if (stageNum < (s - 1))
                {
                    qDebug()<<stageNum << "stageNum";
                    qDebug()<<stageNum+1 << "stageNum+1";
                    secondStageNum = QString::number(stageNum + 1);
                    nextPortNum = getNextPortNum(Topology::intToBase(elementNum * eMCount + eOutputNum, base, nLength), stageNum, base);
                }
                else
                {
                    secondStageNum = " ";
                    nextPortNum = Topology::intToBase(elementNum * eMCount + eOutputNum, base, nLength);   //  отсутствие тасования выходов
                }

                rstrC.append("C*");
                rstrC.append(QString::number(stageNum) + "*");  // ступень
                rstrC.append(Topology::intToBase(elementNum * eMCount + eOutputNum, base, nLength) + "*"); // первая точка соединения
                rstrC.append(secondStageNum + "*");             // номер ступени второй точки
                rstrC.append(nextPortNum);                      // вторая точка соединения (если не последняя ступень, то высчитать номер, иначе такой же
            }
        }
    }
    rstr.append("|"); // отделяет элементы и входы

    for (int outputNum = 0; outputNum < m; outputNum++)
    {
        qDebug()<<"outputNum delta "<< outputNum;
        rstr.append("O" + Topology::intToBase(outputNum, base, nLength));  // номер выхода сети
    }
    rstr.append("|"); // отделяет входы и соединения

    rstr.append(rstrC);
    qDebug()<< "delta" << " " << rstr;
    return rstr;
}

QString Delta::getNextPortNum(const QString portNum, int stageNum, int base) const
{
    bool ok;
    int intPortNum = portNum.toInt(&ok, base);
    int countInStage = getCountInStage(stageNum + 1);
    int _inputCountCross = getInputCountCross();
    int _mod = intPortNum % countInStage;
    int _div = intPortNum / countInStage;

    int nextPort = _mod * _inputCountCross + _div;

    QString nextPortNum = Topology::intToBase(nextPort, base, portNum.length());
    return nextPortNum;

    /*
    bool ok;

    qDebug() << "portNum " << portNum;
    QString nextPortNum = Topology::perfectShuffle(portNum);
    qDebug() << "nextPortNum " << nextPortNum;

    int intNextPort = nextPortNum.toInt(&ok, base);

    qDebug() << "base " << base;
    qDebug() << "intNextPort " << intNextPort;
    qDebug() << "getCountInStage(stageNum) * getOutputCountCross() " << getCountInStage(stageNum) * getOutputCountCross();
    qDebug() << "getCountInStage(stageNum) " << getCountInStage(stageNum);
    qDebug() << "getOutputCountCross() " << getOutputCountCross();
    qDebug() << "intNextPort % (getCountInStage(stageNum) * getOutputCountCross()) " << intNextPort % (getCountInStage(stageNum) * getOutputCountCross());
    qDebug() << "nextPortNum.length() " << nextPortNum.length();
    qDebug() << "Topology::intToBase(intNextPort % (getCountInStage(stageNum) * getOutputCountCross()), base, nextPortNum.length()) " << Topology::intToBase(intNextPort % (getCountInStage(stageNum) * getOutputCountCross()-1), base, nextPortNum.length());

    if (ok)
    {
        nextPortNum = Topology::intToBase(intNextPort % (getCountInStage(stageNum) * getOutputCountCross()), base, nextPortNum.length());
    }

    return nextPortNum;*/
}

QString Delta::getDescription() const
{
    QString description = QString("%1. %2\n").arg("Топология: ").arg("Дельта");

    description += QString("Количество ступеней: %1\n").arg(getStageCount());
    description += QString("Количество входов кроссбара: %1\n").arg(inputCountCross);
    description += QString("Количество выходов кроссбара: %1\n").arg(outputCountCross);

    qDebug()<<"topology "<<description;

    return description;
}

Port* Delta::getOutPortInElement(Port* currentPort, QString controlPacket, QSet<Port*> occupiedPorts)
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

Delta::Delta(int _stage, int _inputCountInCross, int _outputCountInCross)
    //, int stageCount, int selfRouting)
    :Topology(pow(_inputCountInCross, _stage), pow(_outputCountInCross, _stage), _stage, 1, fmax(_inputCountInCross, _outputCountInCross)),
        inputCountCross(_inputCountInCross), outputCountCross(_outputCountInCross)
{
    qDebug()<<"pow(_inputCountInCross, _stage) delta "<< pow(_inputCountInCross, _stage);
    qDebug()<<"pow(_outputCountInCross, _stage) delta "<< pow(_outputCountInCross, _stage);
    QList<int> countInStage;
    int tmpStageOutputCount = getInputCount();

    for(int i = 0; i < _stage; i++)
    {
        countInStage.append(tmpStageOutputCount/inputCountCross);
        tmpStageOutputCount = (tmpStageOutputCount/inputCountCross) * outputCountCross;
    }

    setCountInStage(countInStage);
    referenceString = createReferenceString();
}

bool Delta::checkControlPacket(QString controlPacket)
{
    if (controlPacket.length() != getStageCount())
        return false;

    for (QChar ch : controlPacket)
    {
        if (!ch.isDigit())
            return false;

        int value = ch.digitValue();
        if ((value < 0) || (value >= getOutputCountCross()))
            return false;
    }

    return true;
}
