// ---------------------------------------------------------------------------
// 描述: EraserPathShape 类的实现文件。
//       包含了普通橡皮擦路径的构造、绘制（使用背景色）、边界计算等方法的具体实现。
//       其大部分实现与 FreehandPathShape 非常相似。
// ---------------------------------------------------------------------------

#include "eraserpathshape.h"
#include <QPainter>             // draw 方法需要
#include <QPen>                 // 用于设置画笔
#include <QBrush>               // draw 方法中明确设置为 NoBrush (虽然橡皮擦是用"笔"画的)
#include <QPainterPathStroker>  // getBoundingRect 和 containsPoint 方法需要
#include <QDebug>               // 用于调试输出
#include <QJsonArray>
#include <QJsonObject>

/// @brief EraserPathShape 构造函数的实现。
/// @param points 构成橡皮擦轨迹的初始点集。
/// @param eraserWidth 橡皮擦的宽度（即路径的线宽）。
/// @param eraserColor 橡皮擦用于绘制的颜色，通常是画布的背景色。
///                    此颜色被用作基类的 "borderColor" (shapeColor)，线宽被用作 "penWidth"。
///                    橡皮擦路径不进行“填充”操作。
EraserPathShape::EraserPathShape(const QVector<QPoint> &points, int eraserWidth, const QColor &eraserColor)
    : AbstractShape(ShapeType::NormalEraser, eraserColor, eraserWidth, false, Qt::transparent), // 1. 调用基类构造函数:
    //    - ShapeType::NormalEraser: 指定类型。
    //    - eraserColor: 作为基类的 shapeColor (边框色)，橡皮擦用此颜色绘制路径。
    //    - eraserWidth: 作为基类的 shapePenWidth (线宽)，即橡皮擦粗细。
    //    - false, Qt::transparent: 橡皮擦路径本身不进行额外填充。
    m_points(points) // 2. 初始化存储点的 QVector 成员
{
    // 3. 根据初始的点集构建内部的 QPainterPath 对象。
    buildPath();
    // qDebug() << "EraserPathShape created with" << m_points.size() << "points, width:" << eraserWidth << "color:" << eraserColor.name();
}

/// @brief 私有辅助函数，根据当前的 m_points 点集更新（重建）内部的 m_painterPath 对象。
/// 对于橡皮擦，即使只有一个点（鼠标单击），也应该能擦除一个小区域（通过画笔的 RoundCap 实现）。
void EraserPathShape::buildPath()
{
    m_painterPath = QPainterPath(); // 清空现有路径

    if (m_points.isEmpty()) { // 如果没有点，则路径为空
        return;
    }

    m_painterPath.moveTo(m_points.first()); // 将路径的当前点移动到点集的第一个点
    if (m_points.size() == 1) {
        // 如果只有一个点，画一条长度为0的线到自身，以便 QPainter 使用 RoundCap 时能画出一个圆点。
        m_painterPath.lineTo(m_points.first());
    } else { // 如果有多个点，则连接所有点
        for (int i = 1; i < m_points.size(); ++i) {
            m_painterPath.lineTo(m_points.at(i));
        }
    }
}

// ----------------- eraserpathshape.cpp (请完整替换此函数) -----------------
void EraserPathShape::draw(QPainter *painter)
{
    if (!painter || m_painterPath.isEmpty()) {
        return;
    }

    painter->save(); // 保存状态

    // 同样以路径包围盒的中心为旋转中心
    QPointF center = m_painterPath.boundingRect().center();
    painter->translate(center);
    painter->rotate(m_rotationAngle);
    painter->translate(-center);

    // 设置画笔（橡皮擦的“笔”）
    QPen eraserPen;
    eraserPen.setColor(this->shapeColor);
    eraserPen.setWidth(this->shapePenWidth);
    eraserPen.setCapStyle(Qt::RoundCap);
    eraserPen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(eraserPen);

    painter->setBrush(Qt::NoBrush);

    // 在旋转后的坐标系上绘制路径
    painter->drawPath(m_painterPath);

    painter->restore(); // 恢复状态
}

/// @brief EraserPathShape 类的 getBoundingRect 方法实现。
/// 返回包含整个橡皮擦路径（已考虑其宽度）的最小外接矩形。
QRect EraserPathShape::getBoundingRect() const
{
    if (m_painterPath.isEmpty()) {
        return QRect(); // 空路径返回空矩形
    }
    // 为了获得准确的包围盒（尤其对于有宽度的路径），使用 QPainterPathStroker
    QPainterPathStroker stroker;
    stroker.setWidth(this->shapePenWidth); // 使用橡皮擦的实际宽度
    stroker.setCapStyle(Qt::RoundCap);   // 与绘制时的线帽样式一致
    stroker.setJoinStyle(Qt::RoundJoin); // 与绘制时的连接样式一致
    // createStroke() 返回描边路径，boundingRect() 获取其包围盒 (QRectF)
    return stroker.createStroke(m_painterPath).boundingRect().toAlignedRect();
}

/// @brief EraserPathShape 类的 containsPoint 方法实现。
/// 判断给定点是否在橡皮擦路径的有效点击区域内（考虑到橡皮擦宽度和容差）。
/// 这个方法主要用于当橡皮擦痕迹本身也可以被其他工具（如笔画橡皮擦）操作时的判断。
bool EraserPathShape::containsPoint(const QPoint &point) const
{
    if (m_painterPath.isEmpty()) {
        return false;
    }

    QPainterPathStroker stroker;
    // 设置描边的宽度，基于橡皮擦的实际宽度，并增加一些容差方便点击
    stroker.setWidth(this->shapePenWidth + 4.0); // 例如，增加4像素的点击容差
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);

    QPainterPath strokedPath = stroker.createStroke(m_painterPath); // 生成描边路径

    return strokedPath.contains(point); // 判断点是否在描边路径内
}

/// @brief EraserPathShape 类的 moveBy 方法实现。
/// 将构成橡皮擦路径的所有点都按照给定的偏移量进行平移。
/// (如果橡皮擦的“痕迹”被设计为可移动的，则此方法有用)。
void EraserPathShape::moveBy(const QPoint &offset)
{
    for (QPoint &p : m_points) {
        p += offset;
    }
    buildPath(); // 点集变化后，重新构建 QPainterPath
}

/// @brief EraserPathShape 类的 updateShape 方法实现。
/// 在鼠标拖动绘制橡皮擦路径时调用，用于向路径中添加新的点。
void EraserPathShape::updateShape(const QPoint &point)
{
    addPoint(point); // 直接调用 addPoint 方法
}

/// @brief 向此橡皮擦路径的点集 (m_points) 末尾添加一个新点。
/// 添加点后，会自动调用 buildPath() 来更新内部的 QPainterPath。
/// @param point 要添加的 QPoint。
void EraserPathShape::addPoint(const QPoint &point)
{
    m_points.append(point);
    buildPath();
}

/// @brief 设置构成此橡皮擦路径的完整点集。
/// 这会替换掉现有的所有点。调用此方法后，会自动调用 buildPath()。
/// @param points 包含所有新点的 QVector<QPoint>。
void EraserPathShape::setPoints(const QVector<QPoint> &points)
{
    m_points = points;
    buildPath();
}

QJsonObject EraserPathShape::toJsonObject() const
{
    // 1. 创建基础 JSON 对象并填充通用属性
    QJsonObject json;
    json["type"] = "NormalEraser"; // 类型为 "NormalEraser"
    json["pen_width"] = this->getPenWidth();
    // 对于橡皮擦，其“边框色”就是背景色，我们也记录下来
    json["border_color"] = this->getBorderColor().name();

    // 2. 创建 geometry 对象并填充点集数据
    QJsonObject geometry;
    QJsonArray pointsArray;

    // 遍历 m_points 向量中的所有点
    for(const QPoint &p : m_points){ //
        // 将每个 QPoint(x,y) 转换为 [x, y] 数组，并添加到 pointsArray 中
        pointsArray.append(QJsonArray({p.x(), p.y()}));
    }
    geometry["points"] = pointsArray;

    // 3. 将 geometry 对象放入主对象中
    json["geometry"] = geometry;

    return json;
}

// ----------------- eraserpathshape.cpp (新增函数) -----------------
QPointF EraserPathShape::getCenter() const
{
    // 橡皮擦路径的几何中心，同样是其外包围盒的中心
    return m_painterPath.boundingRect().center();
}

QRectF EraserPathShape::getCoreGeometry() const
{
    return getBoundingRect();
}
