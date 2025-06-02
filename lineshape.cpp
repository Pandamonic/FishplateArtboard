// ---------------------------------------------------------------------------
// 描述: LineShape 类的实现文件。
//       包含了构造函数、绘制方法以及其他从 AbstractShape 继承的虚函数的具体实现。
// ---------------------------------------------------------------------------

#include "lineshape.h"
#include <QPainter>            // draw 方法需要 QPainter
#include <QPainterPath>        // containsPoint 方法使用 QPainterPath
#include <QPainterPathStroker> // containsPoint 方法使用 QPainterPathStroker
#include <QPen>                // draw 方法使用 QPen

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

// LineShape 类的 draw 方法实现
// 此方法定义了如何使用 QPainter 将直线绘制到画布上。
void LineShape::draw(QPainter *painter)
{
    // 安全检查：确保 QPainter 指针有效, 防止空指针出现。
    if (!painter) {
        qWarning("LineShape::draw() - Painter is null!");
        return;
    }

    // 1. 创建并配置画笔 (QPen)
    QPen pen;
    pen.setColor(this->shapeColor);       // 设置画笔颜色 (从基类继承的边框颜色)
    pen.setWidth(this->shapePenWidth);    // 设置画笔宽度 (从基类继承的线宽)
    pen.setCapStyle(Qt::RoundCap);        // 设置线帽样式为圆形，使线条端点看起来更平滑
    // pen.setStyle(Qt::SolidLine);      // (可选) 明确画笔样式，默认为实线
    ///@note 这里未来可以添加一下，换一种画笔（比如说做虚线笔，铅笔，毛笔......）

    // 2. 将配置好的画笔应用到 QPainter
    painter->setPen(pen);

    // 3. 使用 QPainter 的 drawLine 方法绘制直线
    //    参数为 LineShape 自身的成员变量 p1_start 和 p2_end
    painter->drawLine(this->p1_start, this->p2_end);
}

// LineShape 类的 getBoundingRect 方法实现
// 返回包含直线段的最小外接矩形。
QRect LineShape::getBoundingRect() const
{
    // 使用直线的两个端点 p1_start 和 p2_end 来构造一个 QRect。
    // .normalized() 方法确保返回的矩形的左上角坐标值
    // 总是小于等于右下角坐标值，即宽度和高度为正。
    return QRect(p1_start, p2_end).normalized();
}

// LineShape 类的 containsPoint 方法实现
// 判断给定的点是否“足够接近”这条直线段（考虑到线宽）。
bool LineShape::containsPoint(const QPoint &point) const
{
    // 使用 QPainterPath 和 QPainterPathStroker 来创建一个代表“有宽度”的直线路径，
    // 然后判断点是否在该路径内部。这是一种比较精确和通用的方法。

    // 1. 创建一个表示直线中心线的 QPainterPath
    QPainterPath centerLinePath;
    centerLinePath.moveTo(p1_start);
    centerLinePath.lineTo(p2_end);

    // 2. 创建 QPainterPathStroker 对象用于生成描边路径
    QPainterPathStroker stroker;
    // 设置描边的宽度。基于图形的实际线宽，并可以增加一些容差值，
    // 使得用户更容易点击到较细的线条。
    stroker.setWidth(this->shapePenWidth + 4.0); // 例如，增加4个像素的点击容差
    stroker.setCapStyle(Qt::FlatCap); // 对于点击检测，平头线帽可能更符合直线的几何边界

    // 3. 生成描边路径 (stroked path)
    QPainterPath strokedPath = stroker.createStroke(centerLinePath);

    // 4. 判断点是否在生成的描边路径内部
    return strokedPath.contains(point);
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
