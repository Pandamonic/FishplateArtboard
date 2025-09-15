// ----------------- ellipseshape.cpp (重构版) -----------------

#include "ellipseshape.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QJsonArray>
#include <QJsonObject>


EllipseShape::EllipseShape(const QRectF &rect, const QColor &borderColor, int penWidth, bool filled, const QColor &fillColor)
    : AbstractShape(ShapeType::Ellipse, borderColor, penWidth, filled, fillColor),
    m_rect(rect)
{
}

void EllipseShape::draw(QPainter *painter)
{
    if (!painter || m_rect.isNull()) return;

    painter->save();


    QPointF center = m_rect.center();
    painter->translate(center);
    painter->rotate(m_rotationAngle);
    painter->translate(-center);


    painter->setPen(QPen(this->getBorderColor(), this->getPenWidth()));
    if (this->isFilled()) {
        painter->setBrush(QBrush(this->getFillColor()));
    } else {
        painter->setBrush(Qt::NoBrush);
    }


    painter->drawEllipse(m_rect);

    painter->restore();
}

// ----------------- 替换这三个文件中的 getBoundingRect 函数 -----------------
QRect EllipseShape::getBoundingRect() const // "TheShape" 指代 RectangleShape, EllipseShape 或 StarShape
{
    // 如果没有旋转，就用最快的方式返回
    if (m_rotationAngle == 0.0) {
        return m_rect.normalized().toAlignedRect();
    }

    // 1. 创建一个变换矩阵
    QTransform transform;
    // 2. 以图形中心为原点
    transform.translate(m_rect.center().x(), m_rect.center().y());
    // 3. 旋转
    transform.rotate(m_rotationAngle);
    // 4. 将原点移回
    transform.translate(-m_rect.center().x(), -m_rect.center().y());

    // 5. 使用这个矩阵来映射（计算）旋转后的外包围盒
    return transform.mapRect(m_rect.normalized()).toAlignedRect();
}

bool EllipseShape::containsPoint(const QPoint &point) const // "TheShape" 指代 RectangleShape 等
{
    // 如果没有旋转，用最快的方式判断
    if (m_rotationAngle == 0.0) {
        // 这里是你原来的 containsPoint 逻辑
        if (isFilled()) { return m_rect.contains(point); }
        else { /* ... stroker logic ... */ }
    }

    // 1. 创建与 draw() 方法中完全一样的变换矩阵
    QTransform transform;
    transform.translate(m_rect.center().x(), m_rect.center().y());
    transform.rotate(m_rotationAngle);
    transform.translate(-m_rect.center().x(), -m_rect.center().y());

    // 2. 获取该变换的“逆矩阵”
    QTransform inverseTransform = transform.inverted();

    // 3. 将用户点击的点，通过逆矩阵，“反向旋转”回去
    QPointF unrotatedPoint = inverseTransform.map(QPointF(point));

    // 4. 判断这个“反向旋转”后的点，是否在“未旋转”的原始图形内部
    //    这里的逻辑就和我们之前写的 containsPoint 完全一样了
    if (isFilled()) {
        return m_rect.contains(unrotatedPoint);
    } else {
        QPainterPath path;
        path.addEllipse(m_rect);
        QPainterPathStroker stroker;
        stroker.setWidth(this->getPenWidth() + 4.0);
        return stroker.createStroke(path).contains(unrotatedPoint);
    }
}

void EllipseShape::moveBy(const QPoint &offset)
{
    m_rect.translate(offset);
}

void EllipseShape::updateShape(const QPoint &point)
{
    m_rect.setBottomRight(point);
}

void EllipseShape::setGeometry(const QRect &rect)
{
    m_rect = rect;
}

QJsonObject EllipseShape::toJsonObject() const
{
    // 1. 创建一个基础的 JSON 对象
    QJsonObject json;
    json["type"] = "Ellipse"; // 类型为 "Ellipse"
    json["pen_width"] = this->getPenWidth();
    json["border_color"] = this->getBorderColor().name();
    json["is_filled"] = this->isFilled();
    json["fill_color"] = this->getFillColor().name(QColor::HexArgb);

    // 2. 创建一个嵌套的 geometry 对象来存储矩形的位置和尺寸
    QJsonObject geometry;
    geometry["x"] = m_rect.x(); //
    geometry["y"] = m_rect.y();
    geometry["width"] = m_rect.width();
    geometry["height"] = m_rect.height();

    // 3. 将 geometry 对象放入主对象中
    json["geometry"] = geometry;

    return json;
}

QPointF EllipseShape::getCenter() const
{
    return m_rect.center();
}

QRectF EllipseShape::getCoreGeometry() const
{
    return m_rect;
}
