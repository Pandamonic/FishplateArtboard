// ---------------------------------------------------------------------------
// 描述: FreehandPathShape 类的实现文件。
//       包含了自由曲线的构造、绘制、边界计算、点击判断、点集管理等方法的具体实现。
// ---------------------------------------------------------------------------

#include "freehandpathshape.h"
#include <QPainter>             // draw 方法需要
#include <QPen>                 // 用于设置画笔
#include <QPainterPathStroker>  // containsPoint 方法需要
#include <QDebug>               // 用于调试输出 (如果需要)

/// @brief FreehandPathShape 构造函数的实现。
/// @param points 构成自由曲线的初始点集。
/// @param borderColor 路径的颜色。
/// @param penWidth 路径的线宽。
FreehandPathShape::FreehandPathShape(const QVector<QPoint> &points,
                                     const QColor &borderColor, int penWidth)
    : AbstractShape(ShapeType::Freehand, borderColor, penWidth, false, Qt::transparent), // 1. 调用基类构造函数:
    //    - ShapeType::Freehand: 指定类型。
    //    - borderColor, penWidth: 设置边框属性。
    //    - false, Qt::transparent: 自由曲线不进行填充。
    m_points(points) // 2. 初始化存储点的 QVector 成员
{
    // 3. 根据初始的点集构建内部的 QPainterPath 对象，以备绘制和计算使用。
    buildPath();
    // qDebug() << "FreehandPathShape created with" << m_points.size() << "points.";
}

/// @brief 私有辅助函数，根据当前的 m_points 点集更新（重建）内部的 m_painterPath 对象。
/// 当 m_points 列表为空或只有一个点时，m_painterPath 将被清空或设为无效，
/// 因为至少需要两个点才能形成一条可绘制的线段或路径。
void FreehandPathShape::buildPath()
{
    // 清空现有的 QPainterPath，准备重新构建
    m_painterPath = QPainterPath(); // 或者 m_painterPath.clear();

    if (m_points.size() >= 2) { // 至少需要两个点才能构成一条路径
        m_painterPath.moveTo(m_points.first()); // 将路径的当前点移动到点集的第一个点
        for (int i = 1; i < m_points.size(); ++i) {
            m_painterPath.lineTo(m_points.at(i)); // 从当前点向点集中的下一个点添加一条直线段
        }
    } else if (m_points.size() == 1) {
        // 如果只有一个点，可以创建一个包含该点的极小路径，
        // 这样 QPainter 使用 RoundCap 时可以画出一个点。
        m_painterPath.moveTo(m_points.first());
        m_painterPath.lineTo(m_points.first()); // 画一个长度为0的线，配合RoundCap画点
    }
    // 如果 m_points 为空，m_painterPath 也会是空的 (默认构造或 clear() 后)
}

/// @brief FreehandPathShape 类的 draw 方法实现。
/// 使用 QPainter 根据当前路径的属性（颜色、线宽）绘制自由曲线。
void FreehandPathShape::draw(QPainter *painter)
{
    if (!painter || m_painterPath.isEmpty()) { // 安全检查，如果 painter 无效或路径为空则不绘制
        return;
    }

    painter->save(); // 保存 QPainter 当前状态

    // 1. 创建并配置画笔 (QPen)
    QPen pen;
    pen.setColor(this->getBorderColor());   // 使用从基类继承的边框颜色
    pen.setWidth(this->getPenWidth());     // 使用从基类继承的线宽
    // 对于自由画笔，圆形的线帽和连接点能使笔迹看起来更平滑、自然
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);

    // 2. 自由曲线通常不填充，明确设置画刷为 NoBrush
    painter->setBrush(Qt::NoBrush);

    // 3. 绘制预先构建好的 QPainterPath 对象
    painter->drawPath(m_painterPath);

    painter->restore(); // 恢复 QPainter 状态
}

/// @brief FreehandPathShape 类的 getBoundingRect 方法实现。
/// 返回包含整个自由曲线路径的最小外接矩形。
QRect FreehandPathShape::getBoundingRect() const
{
    if (m_painterPath.isEmpty()) {
        return QRect(); // 空路径返回空矩形
    }
    // QPainterPath::controlPointRect() 返回包含所有路径点和控制点的最小矩形(QRectF)。
    // toAlignedRect() 将其转换为像素对齐的整数坐标 QRect。
    return m_painterPath.controlPointRect().toAlignedRect();
}

/// @brief FreehandPathShape 类的 containsPoint 方法实现。
/// 判断给定的点是否在自由曲线路径的有效点击区域内（考虑到线宽和容差）。
bool FreehandPathShape::containsPoint(const QPoint &point) const
{
    if (m_painterPath.isEmpty()) {
        return false; // 空路径不包含任何点
    }

    // 使用 QPainterPathStroker 来创建一个代表“有宽度”的路径描边，
    // 然后判断点是否在该描边路径内部。
    QPainterPathStroker stroker;
    // 设置描边的宽度，基于图形的实际线宽，并增加一些容差方便用户点击。
    stroker.setWidth(this->getPenWidth() + 4.0); // 例如，增加4像素的点击容差
    stroker.setCapStyle(Qt::RoundCap);     // 与绘制时的线帽样式保持一致
    stroker.setJoinStyle(Qt::RoundJoin);   // 与绘制时的连接样式保持一致

    QPainterPath strokedPath = stroker.createStroke(m_painterPath); // 生成描边路径

    return strokedPath.contains(point); // 判断点是否在描边路径内
}

/// @brief FreehandPathShape 类的 moveBy 方法实现。
/// 将构成自由曲线的所有点都按照给定的偏移量进行平移。
void FreehandPathShape::moveBy(const QPoint &offset)
{
    // 遍历 m_points 中的每一个点，并将其坐标加上偏移量
    for (QPoint &p : m_points) { // 使用引用 & 来直接修改容器中的点对象
        p += offset;
    }
    // 点集发生变化后，必须重新构建内部的 QPainterPath
    buildPath();
}

/// @brief FreehandPathShape 类的 updateShape 方法实现。
/// 在鼠标拖动绘制自由曲线时调用，用于向路径中添加新的点。
void FreehandPathShape::updateShape(const QPoint &point)
{
    // 直接调用 addPoint 方法来添加新点并更新路径
    addPoint(point);
}

/// @brief 向此自由曲线的点集 (m_points) 末尾添加一个新点。
/// 添加点后，会自动调用 buildPath() 来更新内部的 QPainterPath。
/// @param point 要添加的 QPoint。
void FreehandPathShape::addPoint(const QPoint &point)
{
    m_points.append(point); // 将新点添加到点集末尾
    buildPath();            // 重新构建 QPainterPath 以反映新的点集
}

/// @brief 设置构成此自由曲线的完整点集。
/// 这会替换掉现有的所有点。调用此方法后，会自动调用 buildPath() 来更新内部的 QPainterPath。
/// @param points 包含所有新点的 QVector<QPoint>。
void FreehandPathShape::setPoints(const QVector<QPoint> &points)
{
    m_points = points; // 用新的点集替换旧的点集
    buildPath();       // 重新构建 QPainterPath
}
