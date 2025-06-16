#include "elementbutton.h"
#include <QMessageLogger>

ElementButton::ElementButton(QWidget *parent): QToolButton(parent)
{}

void ElementButton::mousePressEvent(QMouseEvent *mouseEvent)
{
    QToolButton::mousePressEvent(mouseEvent);
}

void ElementButton::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    QToolButton::mouseReleaseEvent(mouseEvent);
}

void ElementButton::set_Checked(bool _checked) {
    this->_checked = _checked;
    this->setChecked(_checked);
}
