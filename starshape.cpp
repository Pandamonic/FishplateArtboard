// ----------------- starshape.cpp (重构版) -----------------

#include "starshape.h"
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <cmath>
#include <QJsonArray>
#include <QJsonObject>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

StarShape::StarShape(const QRectF &rect, const QColor &borderColor, int penWidth, bool filled, const QColor &fillColor, int numPoints)
    : AbstractShape(ShapeType::Star, borderColor, penWidth, filled, fillColor),
    m_rect(rect),
    m_numPoints(numPoints < 3 ? 3 : numPoints)
{
}

QPolygonF StarShape::calculateStarVertices() const
{
    // [ 新增的关键一行 ]
    // 无论 m_rect 内部状态如何，我们始终在一个标准化的矩形上进行计算
    QRectF bounds = m_rect.normalized();

    QPolygonF starPolygon;

    // [ 下面的所有 m_rect 都要改成 bounds ]
    if (m_numPoints < 3 || bounds.width() <= 0 || bounds.height() <= 0) {
        return starPolygon;
    }

    qreal centerX = bounds.center().x();
    qreal centerY = bounds.center().y();
    qreal outerRadius = qMin(bounds.width() / 2.0, bounds.height() / 2.0);
    qreal innerRadius = outerRadius * 0.45; // 简化因子，你可以调整它来改变星形外观

    qreal angleStep = M_PI / m_numPoints;
    qreal startAngle = -M_PI / 2.0; // 让一个角朝上

    for (int i = 0; i < m_numPoints * 2; ++i) {
        qreal currentRadius = (i % 2 == 0) ? outerRadius : innerRadius;
        qreal currentAngle = startAngle + i * angleStep;
        qreal x = centerX + currentRadius * std::cos(currentAngle);
        qreal y = centerY + currentRadius * std::sin(currentAngle);
        starPolygon << QPointF(x, y);
    }
    return starPolygon;
}


void StarShape::draw(QPainter *painter)
{
    if (!painter || m_rect.isNull() || m_rect.width() <= 0) {
        return;
    }

    painter->save(); // 保存状态

    // 以矩形中心为旋转中心
    QPointF center = m_rect.center();
    painter->translate(center);
    painter->rotate(m_rotationAngle);
    painter->translate(-center);

    // 计算星形的顶点
    QPolygonF starPolygon = calculateStarVertices();
    if (starPolygon.isEmpty()) {
        painter->restore(); // 即使没有顶点也要恢复状态，避免影响后续绘制
        return;
    }

    // 设置画笔和画刷
    painter->setPen(QPen(this->getBorderColor(), this->getPenWidth()));
    if (this->isFilled()) {
        painter->setBrush(QBrush(this->getFillColor()));
    } else {
        painter->setBrush(Qt::NoBrush);
    }

    // 在旋转后的坐标系上绘制星形
    painter->drawPolygon(starPolygon);

    painter->restore(); // 恢复状态
}

// ----------------- 替换这三个文件中的 getBoundingRect 函数 -----------------
QRect StarShape::getBoundingRect() const // "TheShape" 指代 RectangleShape, EllipseShape 或 StarShape
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

// ----------------- 替换这三个文件中的 containsPoint 函数 -----------------
bool StarShape::containsPoint(const QPoint &point) const // "TheShape" 指代 RectangleShape 等
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
        path.addPolygon(calculateStarVertices());
        QPainterPathStroker stroker;
        stroker.setWidth(this->getPenWidth() + 4.0);
        return stroker.createStroke(path).contains(unrotatedPoint);
    }
}

void StarShape::moveBy(const QPoint &offset)
{
    m_rect.translate(offset);
}

void StarShape::updateShape(const QPoint &point)
{
    m_rect.setBottomRight(point);
}

void StarShape::setGeometry(const QRect &rect)
{
    m_rect = rect;
}

QJsonObject StarShape::toJsonObject() const
{
    // 1. 创建基础 JSON 对象并填充通用属性
    QJsonObject json;
    json["type"] = "Star"; // 类型为 "Star"
    json["pen_width"] = this->getPenWidth();
    json["border_color"] = this->getBorderColor().name();
    json["is_filled"] = this->isFilled();
    json["fill_color"] = this->getFillColor().name(QColor::HexArgb);

    // 2. 创建 geometry 对象并填充星形特有的几何属性
    QJsonObject geometry;
    geometry["x"] = m_rect.x(); //
    geometry["y"] = m_rect.y();
    geometry["width"] = m_rect.width();
    geometry["height"] = m_rect.height();
    geometry["num_points"] = m_numPoints; // <-- 保存星形的角点数

    // 3. 将 geometry 对象放入主对象中
    json["geometry"] = geometry;

    return json;
}

QPointF StarShape::getCenter() const
{
    return m_rect.center();
}

QRectF StarShape::getCoreGeometry() const
{
    return m_rect;
}
