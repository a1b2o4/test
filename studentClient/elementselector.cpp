#include "elementselector.h"

ElementSelector::ElementSelector(QWidget *parent)
    : QDockWidget(parent)
{
    setWindowTitle(tr("Element Selector"));    // Устанавливаем заголовок для док-виджета

    mainWidget = new QWidget(this);    // Создаём основной виджет и layout для кнопок
    //mLayout = new QGridLayout(this);
    mLayout = new QVBoxLayout(this);
    mLayout->setAlignment(Qt::AlignTop);

    buttonGroup = new QButtonGroup(this);    // Создаём группу кнопок
    buttonGroup->setExclusive(true);

    connect(buttonGroup, &QButtonGroup::buttonClicked, this, &ElementSelector::handleButtonClick);

    mainWidget->setLayout(mLayout);    // Устанавливаем layout для основного виджета
    setWidget(mainWidget);    // Устанавливаем основной виджет как содержимое QDockWidget

}

void ElementSelector::addButtonCell(QPixmap image, const QString &text)
{
    QToolButton *button = new ElementButton(this);

    QIcon icon(image);

    button->setIcon(icon);
    button->setIconSize(QSize(50, 50));
    button->setCheckable(true);
    button->setText(text);  // Устанавливаем текст кнопки
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);  // Текст под иконкой
    button->setFixedSize(80, 80);  // Задаём фиксированный размер кнопки

    buttonGroup->addButton(button, buttonId);

    mLayout->addWidget(button);//, int(buttonId /  2), int(buttonId %  2));

    buttonId++;
}

void ElementSelector::handleButtonClick(QAbstractButton* button)
{
    QString emitText = "";
    ElementButton* _button = dynamic_cast<ElementButton*>(button);
    bool isChecked =_button->isChecked();
    bool _isChecked =_button->get_Checked();

    buttonGroup->setExclusive(false);

    foreach (QAbstractButton* but, buttonGroup->buttons())
    {
        dynamic_cast<ElementButton*>(but)->set_Checked(false);
    }

    _button->set_Checked(!(_isChecked && isChecked));
    if (!(_isChecked && isChecked)) { emitText = button->text(); }

    buttonGroup->setExclusive(true);

    emit changeState(emitText);
}
