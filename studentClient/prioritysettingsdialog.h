#ifndef PRIORITYSETTINGSDIALOG_H
#define PRIORITYSETTINGSDIALOG_H

#include <QDialog>

class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QDialog;
class SwitchingElement;
class Closa;



class PrioritySettingsDialog : public QDialog
{

    Q_OBJECT

public:

    PrioritySettingsDialog(Closa *topology, int stageNum, int outputCount, QWidget *parent = nullptr);
    QMap<int, int> getUpdatedPriorities() const { return updatedPriorities; }

private:
    Closa *topology;
    int stageNum;
    QMap<int, int> updatedPriorities;
};

#endif // PRIORITYSETTINGSDIALOG_H
