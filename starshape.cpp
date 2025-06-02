// ---------------------------------------------------------------------------
// 描述: StarShape 类的实现文件。
//       包含构造函数、绘制方法 (核心是计算星形顶点)、边界计算、点击判断等。
// ---------------------------------------------------------------------------

#include "starshape.h"
#include <QPainter>
#include <QPainterPath>         // 用于 containsPoint 的精确判断 (边框)
#include <QPainterPathStroker>  // 用于 containsPoint 的精确判断 (边框)
#include <QPen>                 // 用于设置画笔
#include <QBrush>               // 用于设置画刷 (填充)
#include <cmath>                // M_PI 和三角函数 sin, cos (std::sin, std::cos)
#include <QDebug>               // 用于调试输出 (如果需要)

// 定义 M_PI (圆周率)，以防 cmath 中没有默认提供
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

/// @brief StarShape 构造函数的实现。
/// @param point1 定义星形外接矩形的一个角点。
/// @param point2 定义星形外接矩形的另一个角点。
/// @param borderColor 星形的边框颜色。
/// @param penWidth 星形的边框线宽。
/// @param filled 星形是否被填充。
/// @param fillColor 星形的填充颜色。
/// @param numPoints 星形的角数，默认为5。如果小于3，则强制设为3（三角形）。
StarShape::StarShape(const QPoint &point1, const QPoint &point2,
                     const QColor &borderColor, int penWidth,
                     bool filled, const QColor &fillColor, int numPoints)
    : AbstractShape(ShapeType::Star, borderColor, penWidth, filled, fillColor), // 调用基类构造函数
    m_point1(point1),
    m_point2(point2),
    m_numPoints(numPoints < 3 ? 3 : numPoints) // 确保角数至少为3 (三角形)
{
    // qDebug() << "StarShape created with" << m_numPoints << "points.";
}

/// @brief 根据 m_point1 和 m_point2 计算并返回一个标准化的 QRectF。
/// 标准化意味着矩形的左上角 x,y 坐标总是小于等于右下角 x,y 坐标。
/// 使用 QRectF 是为了在计算顶点时有更高的精度。
QRectF StarShape::getNormalizedRectF() const
{
    return QRectF(m_point1, m_point2).normalized();
}

/// @brief 根据给定的外接矩形 `bounds` 和角数 `m_numPoints` 计算星形的顶点。
/// 这是绘制星形的核心几何计算。
/// @param bounds 定义星形绘制范围的 QRectF 对象。
/// @return QPolygonF 对象，包含星形的所有顶点坐标。
QPolygonF StarShape::calculateStarVertices(const QRectF &bounds) const
{
    QPolygonF starPolygon; // 用于存储计算出的顶点

    // 确保角数有效，并且包围盒有效，否则返回空的多边形
    if (m_numPoints < 3 || bounds.width() <= 0 || bounds.height() <= 0) {
        return starPolygon;
    }

    // 计算中心点和半径
    qreal centerX = bounds.center().x();
    qreal centerY = bounds.center().y();
    // 外顶点半径取包围盒宽度和高度中较小值的一半，以确保星形能完整放入
    qreal outerRadius = qMin(bounds.width() / 2.0, bounds.height() / 2.0);
    // 内顶点半径通常是外顶点半径的一个比例，这个比例决定了星形的“尖锐度”
    // 对于标准的五角星，这个比例大约是 sin(18°)/sin(54°) ≈ 0.381966
    // 我们可以用一个可调的比例，例如 0.4 到 0.5 之间
    qreal innerRadiusFactor = 0.45; // 可调整此因子来改变星形外观
    if (m_numPoints == 5) { // 对标准五角星使用更精确的比例
        innerRadiusFactor = 0.381966;
    } else if (m_numPoints == 3 || m_numPoints == 4) { // 三角形或四角星（菱形）没有内凹点
        innerRadiusFactor = 1.0; // 或者直接画正多边形
    }
    qreal innerRadius = outerRadius * innerRadiusFactor;

    // 计算每个顶点（外顶点和内顶点）之间的角度步长
    qreal angleStep = M_PI / m_numPoints; // 这是从一个外顶点到下一个内顶点（或反之）的角度差
        // 所以一个完整的“角”由 2 * angleStep 构成

    // 设置起始角度，通常让一个顶点指向正上方 (-PI/2 或 270度) 或正右方 (0度)
    // -M_PI / 2.0 使得第一个最外层顶点在顶部中央
    qreal startAngle = -M_PI / 2.0;

    // 循环计算 m_numPoints * 2 个顶点 (m_numPoints 个外顶点, m_numPoints 个内顶点)
    for (int i = 0; i < m_numPoints * 2; ++i) {
        // 根据索引 i 的奇偶性交替使用外半径和内半径
        qreal currentRadius = (i % 2 == 0) ? outerRadius : innerRadius;
        // 计算当前顶点的角度
        qreal currentAngle = startAngle + i * angleStep;

        // 使用三角函数计算顶点的 x, y 坐标
        qreal x = centerX + currentRadius * std::cos(currentAngle);
        qreal y = centerY + currentRadius * std::sin(currentAngle);
        starPolygon << QPointF(x, y); // 将计算出的顶点添加到多边形中
    }

    return starPolygon;
}

/// @brief StarShape 类的 draw 方法实现。
/// 计算星形顶点，并使用 QPainter 绘制填充（如果启用）和边框。
void StarShape::draw(QPainter *painter)
{
    if (!painter) { // 安全检查
        qWarning("StarShape::draw() - Painter is null!");
        return;
    }

    QRectF rect = getNormalizedRectF(); // 获取标准化的外接矩形
    // 如果外接矩形无效或尺寸为0，则不进行绘制
    if (rect.isNull() || !rect.isValid() || rect.width() <= 0 || rect.height() <= 0) {
        return;
    }

    QPolygonF starPolygon = calculateStarVertices(rect); // 计算星形的顶点
    if (starPolygon.isEmpty()) { // 如果没有计算出有效顶点，则不绘制
        return;
    }

    painter->save(); // 保存 QPainter 当前状态 (画笔、画刷等)

    // 1. 设置画笔 (用于绘制边框)
    QPen pen;
    pen.setColor(this->getBorderColor()); // 使用基类提供的边框颜色
    pen.setWidth(this->getPenWidth());   // 使用基类提供的线宽
    // pen.setJoinStyle(Qt::MiterJoin); // 对于尖角，MiterJoin效果可能更好
    painter->setPen(pen);

    // 2. 设置画刷 (用于填充图形内部)
    if (this->isFilled()) { // 如果启用了填充
        QBrush brush;
        brush.setColor(this->getFillColor());   // 使用基类提供的填充颜色
        brush.setStyle(Qt::SolidPattern);     // 设置为实心填充
        painter->setBrush(brush);
    } else { // 如果不填充
        painter->setBrush(Qt::NoBrush); // 设置为无填充画刷
    }

    // 3. 绘制星形多边形
    painter->drawPolygon(starPolygon);

    painter->restore(); // 恢复 QPainter 到 save() 之前的状态
}

/// @brief StarShape 类的 getBoundingRect 方法实现。
/// 返回定义星形的外接矩形（转换为整数坐标的 QRect）。
QRect StarShape::getBoundingRect() const
{
    return getNormalizedRectF().toAlignedRect(); // toAlignedRect() 确保像素对齐
}

/// @brief StarShape 类的 containsPoint 方法实现。
/// 判断给定点是否在星形内部（如果填充）或其边框附近（如果只画边框）。
bool StarShape::containsPoint(const QPoint &point) const
{
    QRectF rect = getNormalizedRectF();
    if (rect.isNull() || !rect.isValid() || rect.width() <= 0 || rect.height() <= 0) {
        return false; // 无效的星形不包含任何点
    }

    QPolygonF starPolygon = calculateStarVertices(rect); // 获取星形的顶点
    if (starPolygon.isEmpty()) {
        return false;
    }

    // 将整数点转换为浮点型，以便与 QPolygonF 和 QPainterPath 进行比较
    QPointF pointF(point);

    if (this->isFilled()) { // 如果星形是填充的
        // 使用 QPolygonF::containsPoint 来判断点是否在多边形内部。
        // Qt::OddEvenFill 是标准的填充规则，用于判断点是否在复杂（自相交）多边形内部。
        return starPolygon.containsPoint(pointF, Qt::OddEvenFill);
    } else { // 如果星形只画边框
        // 使用 QPainterPath 和 QPainterPathStroker 来创建一个代表“有宽度”的星形边框路径，
        // 然后判断点是否在该描边路径内部。
        QPainterPath path;
        path.addPolygon(starPolygon); // 从顶点创建路径
        path.closeSubpath();          // 确保路径是闭合的，以便stroker正确工作

        QPainterPathStroker stroker;
        stroker.setWidth(this->getPenWidth() + 4.0); // 线宽加上一些容差，方便点击
        QPainterPath strokedPath = stroker.createStroke(path); // 生成描边路径

        return strokedPath.contains(pointF);
    }
}

/// @brief StarShape 类的 moveBy 方法实现。
/// 将定义星形外接矩形的两个点都按照给定的偏移量进行平移。
void StarShape::moveBy(const QPoint &offset)
{
    m_point1 += offset; // 平移第一个定义点
    m_point2 += offset; // 平移第二个定义点
    // 由于星形的顶点是根据 m_point1 和 m_point2 实时计算的，
    // 所以移动这两个点就相当于移动了整个星形。
}

/// @brief StarShape 类的 updateShape 方法实现。
/// 通常在鼠标拖动创建或修改星形时调用，用于更新其外接矩形的定义。
/// 在我的实现中，是更新定义外接矩形的第二个点 m_point2。
void StarShape::updateShape(const QPoint &point)
{
    m_point2 = point; // 将外接矩形的第二个定义点更新为当前鼠标位置
}
