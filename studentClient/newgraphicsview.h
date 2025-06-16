#ifndef NEWGRAPHICSVIEW_H
#define NEWGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>

class NewGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    NewGraphicsView(QWidget *parent = nullptr);


protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent* event) override;

private:
    bool isDragging;
    QPointF lastPos;
    const int margin = 50;     // Отступ до границы, при котором расширяется сцена
    const int expandSize = 200; // Размер расширения сцены
};

#endif // NEWGRAPHICSVIEW_H
