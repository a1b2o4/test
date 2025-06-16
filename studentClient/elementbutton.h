#ifndef ELEMENTBUTTON_H
#define ELEMENTBUTTON_H

#include <QToolButton>

class ElementButton : public QToolButton
{
    Q_OBJECT

public:
    ElementButton(QWidget *parent);

    void set_Checked(bool _checked);
    bool get_Checked() { return this->_checked;}

private:

    bool _checked = false;

protected:
    void mousePressEvent(QMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent *mouseEvent) override;

};

#endif // ELEMENTBUTTON_H
