// ----------------- ellipseshape.h (重构版) -----------------

#ifndef ELLIPSESHAPE_H
#define ELLIPSESHAPE_H

#include "abstractshape.h"
#include <QRectF>
#include <QJsonObject>

class EllipseShape : public AbstractShape
{
public:
    EllipseShape(const QRectF &rect = QRectF(),
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
    void setGeometry(const QRect &rect) override;
    QJsonObject toJsonObject() const override;
    QPointF getCenter() const override;

private:
    QRectF m_rect;
};

#endif // ELLIPSESHAPE_H
