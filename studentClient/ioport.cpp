/*#include "ioport.h"
#include <cmath>
#include <QString>

IOPort::IOPort(bool _inputPort, QString _numInStage, int _numInElement)
    : inputPort(_inputPort)
{
    port = new Port(!_inputPort, _numInElement, nullptr, _numInStage);
}
*/

/*
QString IOPort::getBinaryPortNum(int base) const {
    if (portNum < 0) return "N/A";

    int precision = std::ceil(std::log2(portNum + 1));
    return QString::number(portNum, base).rightJustified(precision, '0');
}
*/
