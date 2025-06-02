#ifndef LINESHAPE_H
#define LINESHAPE_H

// ---------------------------------------------------------------------------
// 描述: 定义了直线图形类 LineShape，继承自 AbstractShape。
//       负责存储直线的几何数据（起点和终点）并实现其特定的绘制和交互逻辑。
// ---------------------------------------------------------------------------

#include "abstractshape.h" // 包含基类 AbstractShape 的头文件
#include <QPoint>         // 直线图形使用 QPoint 来定义其端点

/// @brief LineShape 类代表画布上的一条直线段。
/// 它继承自 AbstractShape，并实现了绘制直线、计算边界、判断点包含以及移动等操作。
class LineShape : public AbstractShape
{
public:
    /// @brief LineShape 类的构造函数。
    /// @param start 直线的起始点，默认为 QPoint(0,0)。
    /// @param end 直线的结束点，默认为 QPoint(0,0)。
    /// @param borderColor 边框颜色，默认为黑色 (继承自 AbstractShape 的默认值)。
    /// @param penWidth 边框线宽，默认为1像素 (继承自 AbstractShape 的默认值)。
    /// @note 直线不进行填充操作，因此基类的 filled 和 fillColor 参数使用默认值 (false, Qt::transparent)。
    LineShape(const QPoint &start = QPoint(),
              const QPoint &end = QPoint(),
              const QColor &borderColor = Qt::black,
              int penWidth = 1);
    // 注意：这里我们没有显式接收 filled 和 fillColor 参数，
    // 是因为在调用基类构造函数时，我们会为直线直接编码这些值。
    // 另一种设计是也接收这些参数，然后在基类调用时决定。
    // 我们当前在 .cpp 的实现是硬编码为不填充。

    // --- 从 AbstractShape 继承并重写的虚函数 ---

    /// @brief 重写基类的 draw 方法，使用 QPainter 绘制直线。
    /// @param painter 指向 QPainter 对象的指针。
    void draw(QPainter *painter) override;

    /// @brief 重写基类的 getBoundingRect 方法，计算并返回直线的最小外接矩形。
    /// @return QRect 对象，表示直线的 Surrounding box。
    QRect getBoundingRect() const override;

    /// @brief 重写基类的 containsPoint 方法，判断给定点是否在线段的有效点击区域内。
    /// @param point 要测试的点。
    /// @return 如果点在线段上（考虑线宽和容差），则返回 true；否则返回 false。
    bool containsPoint(const QPoint &point) const override;

    /// @brief 重写基类的 moveBy 方法，将直线的两个端点按给定的偏移量移动。
    /// @param offset QPoint 对象，表示在 x 和 y 方向上的移动量。
    void moveBy(const QPoint &offset) override;

    /// @brief 重写基类的 updateShape 方法，通常用于鼠标拖动创建直线时更新其终点。
    /// @param point 当前鼠标指针的位置，用于更新直线的结束点。
    void updateShape(const QPoint &point) override;


    // --- LineShape 特有的公共方法 (可选的 Getters/Setters) ---

    /// @brief 获取直线的起始点。
    /// @return QPoint 对象，表示起始点。
    QPoint getStartPoint() const { return p1_start; }

    /// @brief 获取直线的结束点。
    /// @return QPoint 对象，表示结束点。
    QPoint getEndPoint() const { return p2_end; }

    /// @brief 设置直线的起始点。
    /// @param point 新的起始点。
    void setStartPoint(const QPoint &point) { p1_start = point; }

    /// @brief 设置直线的结束点。
    /// @param point 新的结束点。
    void setEndPoint(const QPoint &point) { p2_end = point; }

private:
    // 存储直线特有的几何数据
    QPoint p1_start; ///< 直线的起始点坐标
    QPoint p2_end;   ///< 直线的结束点坐标
};

#endif // LINESHAPE_H
