// ---------------------------------------------------------------------------
// 描述: RectangleShape 类的实现文件。
//       包含构造函数、绘制方法 (支持边框和填充)、边界计算、点击判断等。
// ---------------------------------------------------------------------------

#include "rectangleshape.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QPainterPath>         // 用于 containsPoint 的边框精确判断
#include <QPainterPathStroker>  // 用于 containsPoint 的边框精确判断
#include <QDebug>               // 用于调试输出

/// @brief RectangleShape 构造函数的实现。
/// @param topLeft 定义矩形的一个角点。
/// @param bottomRight 定义矩形的另一个角点。
/// @param borderColor 矩形的边框颜色。
/// @param penWidth 矩形的边框线宽。
/// @param filled 矩形是否被填充。
/// @param fillColor 矩形的填充颜色。
RectangleShape::RectangleShape(const QPoint &topLeft, const QPoint &bottomRight,
                               const QColor &borderColor, int penWidth,
                               bool filled, const QColor &fillColor)
    : AbstractShape(ShapeType::Rectangle, borderColor, penWidth, filled, fillColor), // 调用基类构造函数
    m_topLeft(topLeft),         // 初始化第一个定义点
    m_bottomRight(bottomRight)  // 初始化第二个定义点
{
    // qDebug() << "RectangleShape created.";
}

/// @brief 根据 m_topLeft 和 m_bottomRight 计算并返回一个标准化的 QRectF。
/// 标准化确保矩形的左上角 x,y 坐标总是小于等于右下角 x,y 坐标，
/// 从而得到一个宽度和高度为正的有效矩形，方便绘制和计算。
/// 使用 QRectF 是为了在几何计算和绘制时获得更高的浮点精度。
QRectF RectangleShape::getNormalizedRectF() const
{
    // QRectF 的构造函数 QRectF(const QPointF &topLeft, const QPointF &bottomRight)
    // 以及其 normalized() 方法可以很好地处理任意两个对角点定义的矩形。
    return QRectF(m_topLeft, m_bottomRight).normalized();
}

/// @brief RectangleShape 类的 draw 方法实现。
/// 使用 QPainter 根据当前矩形的属性（边框颜色、线宽、是否填充、填充颜色）绘制矩形。
void RectangleShape::draw(QPainter *painter)
{
    if (!painter) { // 安全检查
        qWarning("RectangleShape::draw() - Painter is null!");
        return;
    }

    QRectF rectToDraw = getNormalizedRectF(); // 获取标准化的矩形区域进行绘制
    if (rectToDraw.isNull() || !rectToDraw.isValid()) { // 如果矩形无效（例如宽高为0），则不绘制
        return;
    }

    painter->save(); // 保存 QPainter 当前状态 (画笔、画刷等)

    // 1. 设置画笔 (QPen)，用于绘制矩形的边框
    QPen pen;
    pen.setColor(this->getBorderColor()); // 使用从基类继承的边框颜色
    pen.setWidth(this->getPenWidth());   // 使用从基类继承的线宽
    // pen.setStyle(Qt::SolidLine);     // (可选) 明确边框样式
    // pen.setJoinStyle(Qt::MiterJoin); // (可选) 矩形角点通常用 MiterJoin
    painter->setPen(pen);

    // 2. 设置画刷 (QBrush)，用于填充矩形的内部区域
    if (this->isFilled()) { // 如果设置了要填充
        QBrush brush;
        brush.setColor(this->getFillColor()); // 使用从基类继承的填充颜色
        brush.setStyle(Qt::SolidPattern);     // 设置为实心填充模式
        painter->setBrush(brush);
    } else { // 如果不填充
        painter->setBrush(Qt::NoBrush); // 设置为无填充画刷，这样 drawRect 只会画边框
    }

    // 3. 绘制矩形
    // QPainter::drawRect() 会同时使用当前的画笔 (pen) 绘制边框，和当前的画刷 (brush) 填充内部。
    painter->drawRect(rectToDraw);

    painter->restore(); // 恢复 QPainter 到 save() 之前的状态，避免影响后续其他图形的绘制
}

/// @brief RectangleShape 类的 getBoundingRect 方法实现。
/// 对于矩形，其自身的标准化矩形就是它的包围盒。
/// @return QRect 对象，表示此矩形的整数坐标包围盒。
QRect RectangleShape::getBoundingRect() const
{
    return getNormalizedRectF().toAlignedRect(); // toAlignedRect() 将 QRectF 转换为包含它的最小整数 QRect
}

/// @brief RectangleShape 类的 containsPoint 方法实现。
/// 判断给定的点是否在矩形内部（如果填充）或其边框附近（如果只画边框）。
bool RectangleShape::containsPoint(const QPoint &point) const
{
    QRectF rect = getNormalizedRectF(); // 获取标准化的矩形
    if (rect.isNull() || !rect.isValid()) { // 无效矩形不包含任何点
        return false;
    }

    QPointF pointF(point); // 将整数点转换为浮点型进行比较

    if (this->isFilled()) { // 如果矩形是填充的
        // 直接使用 QRectF::contains() 方法判断点是否在矩形区域内部。
        // 这个方法通常不包含矩形的右边和下边（如果 proper=true，默认）。
        // 为了点击选中更容易，可以用 contains(pointF, false) 或者稍微扩大矩形。
        // 或者，如果视觉上填充区域就是整个矩形，那么直接 contains 就可以。
        return rect.contains(pointF);
    } else { // 如果矩形只画边框 (未填充)
        // 判断点是否在矩形的四条边框线上（考虑线宽和容差）。
        // 使用 QPainterPath 和 QPainterPathStroker 是一个较好的方法。
        QPainterPath path;
        path.addRect(rect); // 将矩形添加到路径中

        QPainterPathStroker stroker;
        stroker.setWidth(this->getPenWidth() + 4.0); // 线宽加上一些容差
        stroker.setCapStyle(Qt::SquareCap);          // 矩形端点用方形线帽
        stroker.setJoinStyle(Qt::MiterJoin);         // 矩形角点用斜接

        QPainterPath strokedPath = stroker.createStroke(path); // 生成描边路径

        return strokedPath.contains(pointF); // 判断点是否在描边路径内
    }
}

/// @brief RectangleShape 类的 moveBy 方法实现。
/// 将定义矩形的两个对角点都按照给定的偏移量进行平移。
void RectangleShape::moveBy(const QPoint &offset)
{
    m_topLeft += offset;     // 平移第一个定义点
    m_bottomRight += offset; // 平移第二个定义点
    // 由于矩形的绘制是基于这两个点实时计算的，移动它们即可移动整个矩形。
}

/// @brief RectangleShape 类的 updateShape 方法实现。
/// 通常在鼠标拖动创建或修改矩形时调用，用于更新其第二个定义点。
void RectangleShape::updateShape(const QPoint &point)
{
    m_bottomRight = point; // 将定义矩形的第二个点更新为当前鼠标位置
}
