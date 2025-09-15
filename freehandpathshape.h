// ----------------- freehandpathshape.h (修改后) -----------------
#ifndef FREEHANDPATHSHAPE_H
#define FREEHANDPATHSHAPE_H

#include "abstractshape.h"
#include <QPainterPath>
#include <QVector>
#include <QPoint>
#include <QJsonObject>

class FreehandPathShape : public AbstractShape
{
public:
    FreehandPathShape(const QVector<QPoint> &points,
                      const QColor &borderColor, int penWidth);

    void draw(QPainter *painter) override;
    QRect getBoundingRect() const override;
    bool containsPoint(const QPoint &point) const override;
    void moveBy(const QPoint &offset) override;
    void updateShape(const QPoint &point) override;

    QPointF getCenter() const override; // <--- 添加这一行
    QRectF getCoreGeometry() const override;

    QJsonObject toJsonObject() const override;

    void addPoint(const QPoint &point);
    void setPoints(const QVector<QPoint> &points);
    const QVector<QPoint> &getPoints() const { return m_points; }

private:
    void buildPath();
    QVector<QPoint> m_points;
    QPainterPath m_painterPath;
};

#endif // FREEHANDPATHSHAPE_H
