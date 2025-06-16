#include "variantselectiondialog.h"

VariantSelectionDialog::VariantSelectionDialog(const QStringList& variants, QWidget* parent, QString currentVariant)
    : QDialog(parent), variantList(variants)
{
    setWindowTitle("Выбор варианта");
    setMinimumSize(400, 300);

    // Виджеты
    variantListWidget = new QListWidget(this);
    variantDetails = new QTextEdit(this);
    variantDetails->setReadOnly(true);

    generateButton = new QPushButton("Сгенерировать", this);
    selectButton = new QPushButton("Выбрать", this);
    selectButton->setEnabled(false); // Пока ничего не выбрано

    // Заполняем список номерами вариантов
    for (const QString& variant : variants)
    {
        variantListWidget->addItem(variant.section('-', 0, 0)); // Добавляем номер варианта
    }

    // Выбираем текущий вариант в списке
    if (!currentVariant.isEmpty()) {
        for (int i = 0; i < variantListWidget->count(); ++i) {
            QListWidgetItem* item = variantListWidget->item(i);
            if (item->text() == currentVariant.section('-', 0, 0)) {
                variantListWidget->setCurrentItem(item); // Выбираем элемент списка
                break;
            }
        }
    }

    // Layouts
    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(variantListWidget);

    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->addWidget(variantDetails);
    rightLayout->addWidget(generateButton);
    rightLayout->addWidget(selectButton);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 2);

    setLayout(mainLayout);

    // Сигналы и слоты
    connect(variantListWidget, &QListWidget::currentTextChanged, this, &VariantSelectionDialog::updateVariantDetails);
    connect(selectButton, &QPushButton::clicked, this, &VariantSelectionDialog::accept);
    connect(generateButton, &QPushButton::clicked, this, &VariantSelectionDialog::generateVariant);
}

QString VariantSelectionDialog::getSelectedVariant() const
{
    return selectedVariant;
}
/*
void VariantSelectionDialog::updateVariantDetails(const QString& variantNumber)
{
    for (const QString& variant : variantList)
    {
        if (variant.startsWith(variantNumber + "-"))
        {
            variantDetails->setText(variant);
            selectedVariant = variant;
            selectButton->setEnabled(true);
            return;
        }
    }

    variantDetails->clear();
    selectButton->setEnabled(false);
}*/

void VariantSelectionDialog::updateVariantDetails(const QString& variantNumber)
{
    // Очищаем поле перед обновлением
    variantDetails->clear();


    for (const QString& variant : variantList)
    {
        if (variant.startsWith(variantNumber + "-"))
        {
            variantDetails->setText(variant);
            selectedVariant = variant;
            selectButton->setEnabled(true);
        }
    }

    if (selectedVariant.isEmpty()) {
        variantDetails->setText("Нет данных");
        return;
    }

    QStringList parts = selectedVariant.split('-');
    QString formattedText;

    // Первая часть – номер варианта (если есть)
    if (!parts.isEmpty())
    {
        formattedText += parts[0] + "\n"; // Номер варианта в первой строке
        parts.removeFirst(); // Удаляем его из списка, чтобы не мешал обработке
    }

    // Обрабатываем каждую топологию
    for (const QString& part : parts)
    {
        QStringList subParts = part.split('*'); // Разделяем название топологии и её параметры
        if (subParts.isEmpty()) continue;

        QString topologyName = subParts.takeFirst().trimmed(); // Первое – это название топологии
        formattedText += "\n" + topologyName; // Выводим название с новой строки

        // Определяем параметры по типу топологии
        QStringList paramNames;
        if (topologyName == "Омега" || topologyName == "Баньян" || topologyName == "н-куб")
        {
            paramNames = {"Количество входов сети", "Количество выходов сети"};
        } else if (topologyName == "Дельта")
        {
            paramNames = {"Количество ступеней", "Количество входов кроссбара", "Количество выходов кроссбара"};
        } else if (topologyName == "Клоша")
        {
            paramNames = {
                "Кроссбарами во входной ступени",
                "Кроссбарами в промежуточной ступени",
                "Кроссбарами во выходной ступени",
                "Входами кроссбаров во входной ступени",
                "Выходами кроссбаров во выходной ступени"};
        }

        // Выводим параметры с названиями
        for (int i = 0; i < subParts.size() && i < paramNames.size(); ++i)
        {
            formattedText += "\n" + paramNames[i] + ": " + subParts[i];
        }
        formattedText += "\n";

    }

    // Устанавливаем текст в поле описания
    variantDetails->setText(formattedText);
}


void VariantSelectionDialog::generateVariant()
{

}
