#ifndef ELEMENTSELECTOR_H
#define ELEMENTSELECTOR_H

#include <QDockWidget>
#include <QButtonGroup>
#include <QGridLayout>
#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>

#include "elementbutton.h"

class ElementSelector : public QDockWidget
{
   Q_OBJECT
public:
    ElementSelector(QWidget *parent = nullptr);

    void addButtonCell(QPixmap image, const QString &text);

signals:
    void elementSelected(int elementType);
    void changeState(QString _state);

public slots:
    void handleButtonClick(QAbstractButton* button);

private:
    QButtonGroup *buttonGroup;
    QWidget *mainWidget; // Внутренний виджет для размещения кнопок
    //QGridLayout *mLayout;    // Основной layout
    QVBoxLayout *mLayout;    // Основной layout
    int buttonId = 0;   // следующий свободных номер для кнопки, и количество кнопок в группе
};

#endif // ELEMENTSELECTOR_H
