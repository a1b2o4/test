#include "prioritysettingsdialog.h"
#include "switchingelement.h"
#include "closa.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QSpinBox>
#include <QPushButton>


PrioritySettingsDialog::PrioritySettingsDialog(Closa *topology, int stageNum, int outputCount, QWidget *parent)
    : QDialog(parent), topology(topology), stageNum(stageNum)
{

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Настройка приоритетов выходов (ступень " + QString::number(stageNum) + "):");
    layout->addWidget(label);

    // Получаем текущие приоритеты
    QMap<int, int> priorities = topology->getStagePriorities(stageNum);

    for (int i = 0; i < outputCount; ++i)
    {
        QHBoxLayout *hLayout = new QHBoxLayout();
        QLabel *outLabel = new QLabel("Выход " + QString::number(i) + ": ");
        QSpinBox *prioritySpin = new QSpinBox();
        prioritySpin->setRange(1, outputCount);
        prioritySpin->setValue(priorities[i]);  // Заполняем значением

        connect(prioritySpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this, i](int value) { updatedPriorities[i] = value; });

        hLayout->addWidget(outLabel);
        hLayout->addWidget(prioritySpin);
        layout->addLayout(hLayout);
    }


    QPushButton *okButton = new QPushButton("OK");
    connect(okButton, &QPushButton::clicked, this, &PrioritySettingsDialog::accept);
    layout->addWidget(okButton);

    setLayout(layout);
}
