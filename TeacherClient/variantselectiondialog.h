#ifndef VARIANTSELECTIONDIALOG_H
#define VARIANTSELECTIONDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class VariantSelectionDialog : public QDialog
{

    Q_OBJECT

public:
    VariantSelectionDialog(const QStringList& variants, QWidget* parent = nullptr, QString currentVariant = "");

    QString getSelectedVariant() const;


private slots:
    void updateVariantDetails(const QString& variantNumber);
    void generateVariant();


private:
    QListWidget* variantListWidget;
    QTextEdit* variantDetails;
    QPushButton* generateButton;
    QPushButton* selectButton;

    QStringList variantList;
    QString selectedVariant;
};

#endif // VARIANTSELECTIONDIALOG_H
