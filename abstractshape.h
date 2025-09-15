#ifndef ABSTRACTSHAPE_H
#define ABSTRACTSHAPE_H

#include <QPainter>
#include <QColor>
#include <QPoint>
#include <QRect>
#include <QVector>
#include <QJsonObject>
#include "shared_types.h"

class AbstractShape
{
public:
    AbstractShape(ShapeType type = ShapeType::None,
                  const QColor &borderColor = Qt::black,
                  int penWidth = 1,
                  bool filled = false,
                  const QColor &fillColor = Qt::transparent)
        : shapeType(type),
        shapeColor(borderColor),
        shapePenWidth(penWidth),
        m_isFilled(filled),
        m_shapeFillColor(fillColor),
        m_rotationAngle(0.0)
    {
    }
    virtual ~AbstractShape() {}

    virtual void draw(QPainter *painter) = 0;
    virtual QRect getBoundingRect() const = 0;
    virtual bool containsPoint(const QPoint &point) const = 0;
    virtual void moveBy(const QPoint &offset) = 0;
    virtual void updateShape(const QPoint &point) { Q_UNUSED(point); }

    // setGeometry 是命令模式和视图更新所必需的
    virtual void setGeometry(const QRect &rect) { Q_UNUSED(rect); }

    virtual QPointF getCenter() const = 0;

    // 数据库Json 将图形对象的状态序列化为一个JSON对象。
    virtual QJsonObject toJsonObject() const = 0;

    virtual QRectF getCoreGeometry() const = 0;

    static AbstractShape* fromJsonObject(const QJsonObject &json);


    ShapeType getType() const { return shapeType; }
    QColor getBorderColor() const { return shapeColor; }
    int getPenWidth() const { return shapePenWidth; }
    bool isFilled() const { return m_isFilled; }
    QColor getFillColor() const { return m_shapeFillColor; }
    void setBorderColor(const QColor &color) { shapeColor = color; }
    void setPenWidth(int width) { if (width > 0) shapePenWidth = width; }
    void setFilled(bool filled) { m_isFilled = filled; }
    void setFillColor(const QColor &color) { m_shapeFillColor = color; }
    qreal getRotationAngle() const { return m_rotationAngle; }
    virtual void setRotationAngle(qreal angle) { m_rotationAngle = angle; }

protected:
    ShapeType shapeType;
    QColor shapeColor;
    int shapePenWidth;
    bool m_isFilled;
    QColor m_shapeFillColor;
    qreal m_rotationAngle; // 用于存储图形的旋转角度（单位：度）
};

#endif // ABSTRACTSHAPE_H
