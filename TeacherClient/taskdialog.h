// taskdialog.h
#pragma once

#include <QDialog>
#include <QMap>
#include <QString>

#include "types.h"

class QComboBox;
class QFormLayout;
class QDialogButtonBox;
class QSpinBox;
class QLabel;



class TaskDialog : public QDialog {
    Q_OBJECT

public:
    explicit TaskDialog(QWidget* parent = nullptr);

    QString selectedTopology() const;
    QMap<QString, int> parameters() const;
    QMap<QString, SubtaskScore> subtaskScores() const;

    void setTask(const QString& topology, const QMap<QString, int>& params);

private slots:
    void onTopologyChanged(const QString& topology);

private:
    void updateParameterInputs(const QString& topology);

    QComboBox* topologyBox = nullptr;
    QFormLayout* paramForm = nullptr;
    QFormLayout* scoreForm = nullptr;
    QDialogButtonBox* buttons = nullptr;

    QMap<QString, QSpinBox*> inputFields;
    QMap<QString, QPair<QSpinBox*, QSpinBox*>> scoreFields; // <подзадача, {успех, неуспех}>
};
