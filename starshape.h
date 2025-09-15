// ----------------- starshape.h (重构版) -----------------

#ifndef STARSHAPE_H
#define STARSHAPE_H

#include "abstractshape.h"
#include <QRectF>
#include <QPolygonF>
#include <QJsonObject>

class StarShape : public AbstractShape
{
public:
    StarShape(const QRectF &rect = QRectF(),
              const QColor &borderColor = Qt::black,
              int penWidth = 1,
              bool filled = false,
              const QColor &fillColor = Qt::transparent,
              int numPoints = 5);

    void draw(QPainter *painter) override;
    QRect getBoundingRect() const override;
    QRectF getCoreGeometry() const override;
    bool containsPoint(const QPoint &point) const override;
    void moveBy(const QPoint &offset) override;
    void updateShape(const QPoint &point) override;
    void setGeometry(const QRect &rect) override;
    int getNumPoints() const { return m_numPoints; }
    QJsonObject toJsonObject() const override;
    QPointF getCenter() const override;

private:
    QRectF m_rect;
    int m_numPoints;
    QPolygonF calculateStarVertices() const;
};

#endif // STARSHAPE_H
