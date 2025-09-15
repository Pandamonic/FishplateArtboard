#ifndef GROUPSHAPE_H
#define GROUPSHAPE_H

#include "abstractshape.h"
#include <QList>

class GroupShape : public AbstractShape
{
public:
    explicit GroupShape(const QList<AbstractShape*> &children);
    ~GroupShape() override;

    // 重写基类的所有纯虚函数
    void draw(QPainter *painter) override;
    QRect getBoundingRect() const override;
    bool containsPoint(const QPoint &point) const override;
    void moveBy(const QPoint &offset) override;
    void setRotationAngle(qreal angle) override;
    QJsonObject toJsonObject() const override;
    QPointF getCenter() const override;
    QRectF getCoreGeometry() const override;

    // GroupShape特有的方法
    const QList<AbstractShape*>& getChildren() const;
    QList<AbstractShape*> takeChildren(); // 移交子图形所有权
    void addChildren(const QList<AbstractShape*>& children);

private:
    QList<AbstractShape*> m_children;
};

#endif // GROUPSHAPE_H
