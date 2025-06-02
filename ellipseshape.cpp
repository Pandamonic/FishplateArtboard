// ---------------------------------------------------------------------------
// 描述: EllipseShape 类的实现文件。
//       包含构造函数、绘制方法 (支持边框和填充)、边界计算、点击判断等。
// ---------------------------------------------------------------------------

#include "ellipseshape.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>         // 用于 containsPoint 的精确判断
#include <QPainterPathStroker>  // 用于 containsPoint 的边框精确判断
#include <QDebug>               // 用于调试输出 (如果需要)

/// @brief EllipseShape 构造函数的实现。
/// @param point1 定义椭圆外接矩形的一个角点。
/// @param point2 定义椭圆外接矩形的另一个角点。
/// @param borderColor 椭圆的边框颜色。
/// @param penWidth 椭圆的边框线宽。
/// @param filled 椭圆是否被填充。
/// @param fillColor 椭圆的填充颜色。
EllipseShape::EllipseShape(const QPoint &point1, const QPoint &point2,
                           const QColor &borderColor, int penWidth,
                           bool filled, const QColor &fillColor)
    : AbstractShape(ShapeType::Ellipse, borderColor, penWidth, filled, fillColor), // 调用基类构造函数
    m_point1(point1),       // 初始化第一个定义点
    m_point2(point2)        // 初始化第二个定义点
{
    // qDebug() << "EllipseShape created.";
}

/// @brief 根据 m_point1 和 m_point2 计算并返回一个标准化的 QRectF (浮点型矩形)。
/// 标准化确保矩形的左上角 x,y 坐标总是小于等于右下角 x,y 坐标，
/// 从而得到一个宽度和高度为正的有效矩形，方便绘制和几何计算。
QRectF EllipseShape::getNormalizedRectF() const
{
    return QRectF(m_point1, m_point2).normalized();
}

/// @brief EllipseShape 类的 draw 方法实现。
/// 使用 QPainter 根据当前椭圆的属性（边框颜色、线宽、是否填充、填充颜色）绘制椭圆。
void EllipseShape::draw(QPainter *painter)
{
    if (!painter) { // 安全检查
        qWarning("EllipseShape::draw() - Painter is null!");
        return;
    }

    QRectF rectToDraw = getNormalizedRectF(); // 获取定义椭圆的标准外接矩形
    if (rectToDraw.isNull() || !rectToDraw.isValid()) { // 如果矩形无效，则不绘制
        return;
    }

    painter->save(); // 保存 QPainter 当前状态

    // 1. 设置画笔 (QPen)，用于绘制椭圆的边框
    QPen pen;
    pen.setColor(this->getBorderColor()); // 使用从基类继承的边框颜色
    pen.setWidth(this->getPenWidth());   // 使用从基类继承的线宽
    painter->setPen(pen);

    // 2. 设置画刷 (QBrush)，用于填充椭圆的内部区域
    if (this->isFilled()) { // 如果设置了要填充
        QBrush brush;
        brush.setColor(this->getFillColor()); // 使用从基类继承的填充颜色
        brush.setStyle(Qt::SolidPattern);     // 设置为实心填充模式
        painter->setBrush(brush);
    } else { // 如果不填充
        painter->setBrush(Qt::NoBrush); // 设置为无填充画刷
    }

    // 3. 绘制椭圆
    // QPainter::drawEllipse() 会使用当前的画笔绘制边框，使用当前的画刷填充内部。
    painter->drawEllipse(rectToDraw); // 传入 QRectF 以便绘制平滑椭圆

    painter->restore(); // 恢复 QPainter 到 save() 之前的状态
}

/// @brief EllipseShape 类的 getBoundingRect 方法实现。
/// 返回定义椭圆的外接矩形（转换为整数坐标的 QRect）。
QRect EllipseShape::getBoundingRect() const
{
    return getNormalizedRectF().toAlignedRect(); // toAlignedRect() 确保像素对齐
}

/// @brief EllipseShape 类的 containsPoint 方法实现。
/// 判断给定的点是否在椭圆内部（如果填充）或其边框附近（如果只画边框）。
bool EllipseShape::containsPoint(const QPoint &point) const
{
    QRectF rect = getNormalizedRectF(); // 获取标准外接矩形
    if (rect.isNull() || !rect.isValid()) {
        return false; // 无效椭圆不包含任何点
    }

    // 创建一个表示此椭圆的 QPainterPath，用于精确的点击检测
    QPainterPath path;
    path.addEllipse(rect); // 将椭圆添加到路径中

    QPointF pointF(point); // 将整数点转换为浮点型进行比较

    if (this->isFilled()) { // 如果椭圆是填充的
        // 使用 QPainterPath::contains() 方法判断点是否在椭圆路径所围成的内部区域。
        return path.contains(pointF);
    } else { // 如果椭圆只画边框 (未填充)
        // 使用 QPainterPathStroker 来创建一个代表“有宽度”的椭圆边框路径，
        // 然后判断点是否在该描边路径内部。
        QPainterPathStroker stroker;
        stroker.setWidth(this->getPenWidth() + 4.0); // 线宽加上一些容差，方便点击
        // stroker.setCapStyle(...); // 对于闭合路径，CapStyle 通常不那么重要
        // stroker.setJoinStyle(...); // 对于椭圆这种平滑曲线，JoinStyle 也不太重要
        QPainterPath strokedPath = stroker.createStroke(path); // 生成描边路径

        return strokedPath.contains(pointF); // 判断点是否在描边路径内
    }
}

/// @brief EllipseShape 类的 moveBy 方法实现。
/// 将定义椭圆外接矩形的两个对角点都按照给定的偏移量进行平移。
void EllipseShape::moveBy(const QPoint &offset)
{
    m_point1 += offset;     // 平移第一个定义点
    m_point2 += offset;     // 平移第二个定义点
    // 由于椭圆的绘制是基于这两个点实时计算外接矩形的，移动它们即可移动整个椭圆。
}

/// @brief EllipseShape 类的 updateShape 方法实现。
/// 通常在鼠标拖动创建或修改椭圆时调用，用于更新其外接矩形的第二个定义点。
void EllipseShape::updateShape(const QPoint &point)
{
    m_point2 = point; // 将外接矩形的第二个定义点更新为当前鼠标位置
}
