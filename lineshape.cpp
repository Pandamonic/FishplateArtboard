// ---------------------------------------------------------------------------
// 描述: LineShape 类的实现文件。
//       包含了构造函数、绘制方法以及其他从 AbstractShape 继承的虚函数的具体实现。
// ---------------------------------------------------------------------------

#include "lineshape.h"
#include <QPainter>            // draw 方法需要 QPainter
#include <QPainterPath>        // containsPoint 方法使用 QPainterPath
#include <QPainterPathStroker> // containsPoint 方法使用 QPainterPathStroker
#include <QPen>                // draw 方法使用 QPen
#include <QJsonArray>
#include <QJsonObject>

// LineShape 类的构造函数实现
LineShape::LineShape(const QPoint &start, const QPoint &end,
                     const QColor &borderColor, int penWidth)
    : AbstractShape(ShapeType::Line, borderColor, penWidth, false, Qt::transparent),
    // 1. 调用基类 AbstractShape 的构造函数：
    //    - ShapeType::Line: 明确指定图形类型为直线。
    //    - borderColor, penWidth: 传递边框颜色和线宽。
    //    - false, Qt::transparent: 直线通常不进行填充，所以显式传递不填充状态和透明填充色。
    p1_start(start),  // 2. 初始化直线特有的成员变量：起始点
    p2_end(end)       // 3. 初始化直线特有的成员变量：结束点
{
    // 构造函数体。
    // 对于 LineShape，所有必要的初始化已在成员初始化列表中完成。
    // qDebug() << "LineShape object created. Start:" << p1_start << "End:" << p2_end;
}


void LineShape::draw(QPainter *painter)
{
    if (!painter) {
        qWarning("LineShape::draw() - Painter is null!");
        return;
    }

    painter->save(); // 保存状态

    // 计算直线的中心点作为旋转中心
    QPointF center = (p1_start.toPointF() + p2_end.toPointF()) / 2.0;
    painter->translate(center);
    painter->rotate(m_rotationAngle);
    painter->translate(-center);

    // 设置画笔
    QPen pen;
    pen.setColor(this->shapeColor);
    pen.setWidth(this->shapePenWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);

    // 在旋转后的坐标系上绘制直线
    painter->drawLine(this->p1_start, this->p2_end);

    painter->restore(); // 恢复状态
}

QRect LineShape::getBoundingRect() const
{
    if (m_rotationAngle == 0.0) return QRect(p1_start, p2_end).normalized();
    QTransform t;
    QPointF center = (p1_start.toPointF() + p2_end.toPointF()) / 2.0;
    t.translate(center.x(), center.y());
    t.rotate(m_rotationAngle);
    t.translate(-center.x(), -center.y());
    return t.mapRect(QRect(p1_start, p2_end));
}

bool LineShape::containsPoint(const QPoint &point) const
{
    QTransform t;
    QPointF center = (p1_start.toPointF() + p2_end.toPointF()) / 2.0;
    t.translate(center.x(), center.y());
    t.rotate(m_rotationAngle);
    t.translate(-center.x(), -center.y());

    QPointF unrotatedPoint = t.inverted().map(QPointF(point));

    QPainterPath path;
    path.moveTo(p1_start);
    path.lineTo(p2_end);
    QPainterPathStroker stroker;
    stroker.setWidth(this->getPenWidth() + 4.0);
    return stroker.createStroke(path).contains(unrotatedPoint);
}

// LineShape 类的 moveBy 方法实现
// 将直线的两个端点都按照给定的偏移量进行平移。
void LineShape::moveBy(const QPoint &offset)
{
    p1_start += offset; // 起点坐标加上偏移量
    p2_end += offset;   // 终点坐标加上偏移量
}

// LineShape 类的 updateShape 方法实现
// 通常在鼠标拖动创建或修改直线时调用，用于更新直线的某个端点。
// 在我们的实现中，通常是更新结束点 p2_end。
void LineShape::updateShape(const QPoint &point)
{
    p2_end = point; // 将直线的结束点更新为当前鼠标位置
}



QJsonObject LineShape::toJsonObject() const
{
    QJsonObject json;
    json["type"] = "Line";
    json["pen_width"] = this->getPenWidth();
    json["border_color"] = this->getBorderColor().name();

    QJsonObject geometry;
    // 将 QPoint(x,y) 转换为 [x, y] 数组
    geometry["p1"] = QJsonArray({p1_start.x(), p1_start.y()});
    geometry["p2"] = QJsonArray({p2_end.x(), p2_end.y()});
    json["rotation"] = m_rotationAngle;

    json["geometry"] = geometry;
    return json;
}

QPointF LineShape::getCenter() const
{
    return (p1_start.toPointF() + p2_end.toPointF()) / 2.0;
}

QRectF LineShape::getCoreGeometry() const
{
    return getBoundingRect();
}
