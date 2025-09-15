#include "rectangleshape.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QJsonArray>


RectangleShape::RectangleShape(const QRectF &rect, const QColor &borderColor, int penWidth, bool filled, const QColor &fillColor)
    : AbstractShape(ShapeType::Rectangle, borderColor, penWidth, filled, fillColor),
    m_rect(rect)
{
}

void RectangleShape::draw(QPainter *painter)
{
    if (!painter || m_rect.isNull()) return;

    painter->save(); // 1. 保存QPainter当前状态（像创建一个存档）

    // 计算旋转中心
    QPointF center = m_rect.center();

    // 2. 将坐标系原点移动到矩形中心
    painter->translate(center);
    // 3. 旋转坐标系
    painter->rotate(m_rotationAngle);
    // 4. 将坐标系原点移回原来的位置
    painter->translate(-center);

    // 5. 在这个已经被旋转的坐标系上，像平常一样画矩形
    QPen pen(this->getBorderColor(), this->getPenWidth());
    painter->setPen(pen);
    if (this->isFilled()) {
        painter->setBrush(QBrush(this->getFillColor()));
    } else {
        painter->setBrush(Qt::NoBrush);
    }
    painter->drawRect(m_rect);

    painter->restore(); // 6. 恢复到存档时的状态，以免影响其他图形的绘制
}

// ----------------- 替换这三个文件中的 getBoundingRect 函数 -----------------
QRect RectangleShape::getBoundingRect() const // "TheShape" 指代 RectangleShape, EllipseShape 或 StarShape
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

bool RectangleShape::containsPoint(const QPoint &point) const // "TheShape" 指代 RectangleShape 等
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
        path.addRect(m_rect); // 对于椭圆是 addEllipse
        QPainterPathStroker stroker;
        stroker.setWidth(this->getPenWidth() + 4.0);
        return stroker.createStroke(path).contains(unrotatedPoint);
    }
}

void RectangleShape::moveBy(const QPoint &offset)
{
    m_rect.translate(offset);
}

void RectangleShape::updateShape(const QPoint &point)
{
    m_rect.setBottomRight(point);
}

void RectangleShape::setGeometry(const QRect &rect)
{
    m_rect = rect;
}


QJsonObject RectangleShape::toJsonObject() const
{
    QJsonObject json;
    json["type"] = "Rectangle";
    json["pen_width"] = this->getPenWidth();
    json["border_color"] = this->getBorderColor().name();
    json["is_filled"] = this->isFilled();
    json["fill_color"] = this->getFillColor().name(QColor::HexArgb);

    QJsonObject geometry;
    geometry["x"] = m_rect.x();
    geometry["y"] = m_rect.y();
    geometry["width"] = m_rect.width();
    geometry["height"] = m_rect.height();
    json["rotation"] = m_rotationAngle;
    json["geometry"] = geometry;

    return json;
}

QPointF RectangleShape::getCenter() const
{
    return m_rect.center();
}

QRectF RectangleShape::getCoreGeometry() const
{
    return m_rect;
}
