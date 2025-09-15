// ----------------- eraserpathshape.h (修改后) -----------------
#ifndef ERASERPATHSHAPE_H
#define ERASERPATHSHAPE_H

#include "abstractshape.h"
#include <QPainterPath>
#include <QVector>
#include <QPoint>
#include <QJsonObject>

class EraserPathShape : public AbstractShape
{
public:
    EraserPathShape(const QVector<QPoint> &points, int eraserWidth, const QColor &eraserColor);

    void draw(QPainter *painter) override;
    QRect getBoundingRect() const override;
    QRectF getCoreGeometry() const override;
    bool containsPoint(const QPoint &point) const override;
    void moveBy(const QPoint &offset) override;
    void updateShape(const QPoint &point) override;

    QPointF getCenter() const override; // <--- 添加这一行

    QJsonObject toJsonObject() const override;

    void addPoint(const QPoint &point);
    void setPoints(const QVector<QPoint> &points);
    const QVector<QPoint> &getPoints() const { return m_points; }

private:
    void buildPath();
    QVector<QPoint> m_points;
    QPainterPath m_painterPath;
};

#endif // ERASERPATHSHAPE_H
