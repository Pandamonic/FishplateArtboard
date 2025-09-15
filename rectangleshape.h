#ifndef RECTANGLESHAPE_H
#define RECTANGLESHAPE_H

#include "abstractshape.h"
#include <QRectF>
#include <QJsonObject>

class RectangleShape : public AbstractShape
{
public:
    RectangleShape(const QRectF &rect = QRectF(),
                   const QColor &borderColor = Qt::black,
                   int penWidth = 1,
                   bool filled = false,
                   const QColor &fillColor = Qt::transparent);

    void draw(QPainter *painter) override;
    QRect getBoundingRect() const override;
    QRectF getCoreGeometry() const override;
    bool containsPoint(const QPoint &point) const override;
    void moveBy(const QPoint &offset) override;
    void updateShape(const QPoint &point) override;
    // resize 函数已从 shape 中移除
    void setGeometry(const QRect &rect) override;
    QPointF getCenter() const override;

    QJsonObject toJsonObject() const override;

private:
    QRectF m_rect;
};

#endif // RECTANGLESHAPE_H
