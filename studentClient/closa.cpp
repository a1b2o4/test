#include "closa.h"
#include "port.h"
#include "switchingelement.h"
#include "connection.h"


QString Closa::createReferenceString() const
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

    int eNCount;// = getInputCountCross();
    int eMCount;// = getOutputCountCross();

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

        eNCount = countInputInCrossInStage[stageNum];
        eMCount = countOutputInCrossInStage[stageNum];

        int base = 2;
        //int nLength = std::ceil(log10(fmax(eNCount,eMCount)) / log10(base));
        int nLength = std::ceil(log10(eNCount * getCountInStage(stageNum)) / log10(base));
        qDebug()<<"getCountInStage(stageNum) closa "<< getCountInStage(stageNum);
        qDebug()<<"log10(eNCount * getCountInStage(stageNum)) closa "<< log10(eNCount * getCountInStage(stageNum));
        qDebug()<<"log10(base) closa "<< log10(base);
        qDebug()<<"log10(eNCount * getCountInStage(stageNum)) / log10(base) closa "<< log10(eNCount * getCountInStage(stageNum)) / log10(base);
        qDebug()<<"std::ceil(log10(eNCount * getCountInStage(stageNum)) / log10(base)) closa "<< std::ceil(log10(eNCount * getCountInStage(stageNum)) / log10(base));
        int mLength = std::ceil(log10(eMCount * getCountInStage(stageNum)) / log10(base));
        qDebug()<<"nLength closa "<< nLength;
        qDebug()<<"nLength closa "<< nLength;
        qDebug()<<"nLength closa "<< nLength;
        qDebug()<<"nLength closa "<< nLength;
        qDebug()<<"mLength closa "<< mLength;

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
                qDebug()<<"nLength "<< nLength;
                rstr.append(QString::number(stageNum) + "*");   // ступень
                rstr.append(QString::number(elementNum));       // номер элемента для тек входа в ступени
            }

            for (int eOutputNum = 0; eOutputNum < eMCount; eOutputNum++)
            {
                rstr.append("o" + QString::number(eOutputNum) + "*");  // номер выхода в элементе
                rstr.append(Topology::intToBase(elementNum * eMCount + eOutputNum, base, mLength) + "*");   // номер выхода в ступени
                rstr.append(QString::number(stageNum) + "*");  // ступень
                rstr.append(QString::number(elementNum));      // номер элемента для тек выхода в ступени

                if (stageNum < (s - 1))
                {
                    qDebug()<<stageNum << "stageNum";
                    qDebug()<<stageNum+1 << "stageNum+1";
                    secondStageNum = QString::number(stageNum + 1);
                    nextPortNum = getNextPortNum(Topology::intToBase(elementNum * eMCount + eOutputNum, base, mLength), stageNum, base);
                }
                else
                {
                    secondStageNum = " ";
                    nextPortNum = Topology::intToBase(elementNum * eMCount + eOutputNum, base, mLength);   //  отсутствие тасования выходов
                }

                rstrC.append("C*");
                rstrC.append(QString::number(stageNum) + "*");  // ступень
                rstrC.append(Topology::intToBase(elementNum * eMCount + eOutputNum, base, mLength) + "*"); // первая точка соединения
                rstrC.append(secondStageNum + "*");             // номер ступени второй точки
                rstrC.append(nextPortNum);                      // вторая точка соединения (если не последняя ступень, то высчитать номер, иначе такой же
            }
        }
    }
    rstr.append("|"); // отделяет элементы и входы

    int mLength = std::ceil(log10(getOutputCount()) / log10(base));
    for (int outputNum = 0; outputNum < m; outputNum++)
    {
        rstr.append("O" + Topology::intToBase(outputNum, base, mLength));  // номер выхода сети
    }
    rstr.append("|"); // отделяет входы и соединения

    rstr.append(rstrC);
    qDebug()<< "closa " << " " << rstr;
    return rstr;
}

QString Closa::getNextPortNum(const QString portNum, int stageNum, int base) const
{
    bool ok;
    int intPortNum = portNum.toInt(&ok, base);
    int countInStage = getCountInStage(stageNum + 1);
    int _inputCountCross = getCountInStage(stageNum);
    int _mod = intPortNum % countInStage;
    int _div = intPortNum / countInStage;

    int nextPort = _mod * _inputCountCross + _div;

    QString nextPortNum = Topology::intToBase(nextPort, base, portNum.length());
    return nextPortNum;
}

Port* Closa::getOutPortInElement(Port* currentPort, QString controlPacket, QSet<Port*> occupiedPorts)
{   
    SwitchingElement* SwEl = currentPort->getSwitchingElement();
    qDebug() << "SwEl " << !SwEl;
    if (!SwEl) return nullptr;                          // Проверка, что элемент существует

    int stage = SwEl->getStage().toInt();               // Получаем текущую ступень
    QList<Port*> outputPorts = SwEl->getOutputPorts();  // Список выходных портов
    if (outputPorts.isEmpty()) return nullptr;          // Если нет выходов, возвращаем nullptr

    //------ получение outputIndex
    if (stage == 0)
    {
        qDebug() << "stage == 0 " << stage;
        // Получаем карту приоритетов для данной ступени
        QMap<int, int> priorityMap = getStagePriorities(stage);

        Port* selectedPort = currentPort->getOutputConnections()[0]->getEndItem();  // Выбранный выходной порт

        // Проверяем, не занят ли он
        if (!occupiedPorts.contains(selectedPort)) return selectedPort;

        // Если выбранный выход занят, ищем свободный порт с наивысшим приоритетом
        int maxPriority = -1;
        Port* bestPort = nullptr;

        for (auto it = priorityMap.begin(); it != priorityMap.end(); ++it)
        {
            qDebug() << "maxPriority " << maxPriority;
            int portIndex = it.key();
            int priority = it.value();

            if (portIndex < outputPorts.size())
            {
                Port* port = outputPorts[portIndex];

                if (!occupiedPorts.contains(port) && priority > maxPriority)
                {
                    maxPriority = priority;
                    bestPort = port;
                }
            }
        }
        qDebug() << "bestPort " << bestPort->getFields();
        return bestPort;  // Возвращаем порт с наивысшим приоритетом или nullptr, если нет доступных портов

    }

    SwEl->setControlPacket(controlPacket);
    qDebug() << "setControlPacket " << controlPacket;

    stage = SwEl->getStage().toInt();  // Получаем текущую ступень

    //------ получение outputIndex
    int outputIndex = controlPacket[stage].digitValue();
    //------ получение outputIndex

    qDebug() << "outputIndex " << outputIndex;
    currentPort = SwEl->getOutputPorts()[outputIndex];
    qDebug() << "currentPort " << !currentPort;

    return currentPort;

}

QString Closa::getDescription() const
{
    QString description = QString("%1. %2\n").arg("Топология: ").arg("Клоша");

    description += QString("Кроссбаров во входной ступени: %1\n").arg(inputCrossCount);
    description += QString("Кроссбаров во промежуточной ступени: %1\n").arg(outputCrossCount);
    description += QString("Кроссбаров во выходной ступени: %1\n").arg(midCrossCount);
    description += QString("Входами кроссбаров во входной ступени: %1\n").arg(inputInCrossInInput);
    description += QString("Выходами кроссбаров во выходной ступени: %1\n").arg(outputInCrossInOutput);

    qDebug()<<"topology "<<description;

    return description;
}

Closa::Closa(int _inputCrossCount, int _outputCrossCount, int _midCrossCount, int _inputInCrossInInput, int _outputInCrossInOutput)
    :Topology(_inputCrossCount*_inputInCrossInInput, _outputCrossCount*_outputInCrossInOutput, 3, 0, true),
    inputCrossCount(_inputCrossCount),
    outputCrossCount(_outputCrossCount),
    midCrossCount(_midCrossCount),
    inputInCrossInInput(_inputInCrossInInput),
    outputInCrossInOutput(_outputInCrossInOutput)
{
    QList<int> countInStage;
    countInStage.append(_inputCrossCount);
    countInStage.append(_midCrossCount);
    countInStage.append(_outputCrossCount);


    countInputInCrossInStage.append(_inputInCrossInInput);
    countInputInCrossInStage.append(_inputCrossCount);
    countInputInCrossInStage.append(_midCrossCount);

    countOutputInCrossInStage.append(_midCrossCount);
    countOutputInCrossInStage.append(_outputCrossCount);
    countOutputInCrossInStage.append(_outputInCrossInOutput);

    setCountInStage(countInStage);
    referenceString = createReferenceString();

    //for (int i = 0; i < getStageCount(); i++)
    for (int i = 0; i < getStageCount(); i++)
    {
        for (int j = 0; j < countOutputInCrossInStage[i]; j++)
        {

            stagePriorities[i][j] = 1;  // Значение по умолчанию
        }
    }
}

//клоша - нет тасования входов, нет тасования выходов, переключается по уу если путь занят

bool Closa::checkControlPacket(QString controlPacket)
{
    if (controlPacket.length() != getStageCount() - 1)
        return false;

    for (int i = 0; i < controlPacket.length(); ++i)
    {
        QChar ch = controlPacket[i];

        if (!ch.isDigit())
            return false;

        int value = ch.digitValue();

        if (value < 0 || value >= countOutputInCrossInStage[i + 1])
            return false;
    }

    return true;
}


