#include "newgraphicsview.h"

#include <QApplication>

NewGraphicsView::NewGraphicsView(QWidget *parent)
{
    setDragMode(QGraphicsView::ScrollHandDrag);  // Включаем стандартное поведение для перемещения
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


void NewGraphicsView::enterEvent(QEnterEvent *event)
{
    QGraphicsView::enterEvent(event);
    qDebug() << "setOverrideCursor";
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void NewGraphicsView::leaveEvent(QEvent* event)
{
    // Сбрасываем курсор в стандартное состояние
    QApplication::restoreOverrideCursor();
    qDebug() << "restoreOverrideCursor";
    QGraphicsView::leaveEvent(event);
}

void NewGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        lastPos = event->pos();  // Сохраняем начальную позицию при нажатии
    }
    QGraphicsView::mousePressEvent(event);
}

void NewGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QRectF visibleRect = this->viewport()->rect(); // Видимая область сцены
    QRectF sceneBounds = this->scene()->sceneRect();
    QRectF visibleSceneRect = mapToScene(visibleRect.toRect()).boundingRect();
    //qDebug() << "sceneBounds: " << sceneBounds;
    //qDebug() << "visibleSceneRect: " << visibleSceneRect;
    //qDebug() << "visible w, h: " << visibleRect.width() << " " << visibleRect.height();

 /*   if (isDragging) {
        QPointF delta = mapToScene(event->pos()) - mapToScene(lastPos.toPoint());
        lastPos = event->pos();
        translate(delta.x(), delta.y());

        // Получаем границы сцены
        QRectF sceneBounds = this->scene()->sceneRect();
        QRectF visibleRect = this->viewport()->rect(); // Видимая область сцены

        // Преобразуем видимую область в мировые координаты
        QRectF visibleSceneRect = mapToScene(visibleRect.toRect()).boundingRect();

        qDebug() << "sceneBounds: " << sceneBounds;
        qDebug() << "visibleSceneRect: " << visibleSceneRect;

        // Проверяем, достигла ли сцена границы видимой области
        if (sceneBounds.right() < visibleSceneRect.right()) {
            // Увеличиваем сцену, если достигнут правый край
            scene()->setSceneRect(sceneBounds.adjusted(0, 0, 100, 0)); // Расширяем сцену вправо
        }

        if (sceneBounds.left() > visibleSceneRect.left()) {
            // Увеличиваем сцену, если достигнут левый край
            scene()->setSceneRect(sceneBounds.adjusted(-100, 0, 0, 0)); // Расширяем сцену влево
        }

        if (sceneBounds.bottom() < visibleSceneRect.bottom()) {
            // Увеличиваем сцену, если достигнут нижний край
            scene()->setSceneRect(sceneBounds.adjusted(0, 0, 0, 100)); // Расширяем сцену вниз
        }

        if (sceneBounds.top() > visibleSceneRect.top()) {
            // Увеличиваем сцену, если достигнут верхний край
            scene()->setSceneRect(sceneBounds.adjusted(0, -100, 0, 0)); // Расширяем сцену вверх
        }

    }

*/
    QGraphicsView::mouseMoveEvent(event);
}

void NewGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    isDragging = false;  // Останавливаем перемещение
    QGraphicsView::mouseReleaseEvent(event);
}
