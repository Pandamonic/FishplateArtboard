#ifndef RECTANGLESHAPE_H
#define RECTANGLESHAPE_H

// ---------------------------------------------------------------------------
// 描述: 定义了矩形图形类 RectangleShape，继承自 AbstractShape。
//       负责存储矩形的几何数据（通过两个对角点定义）
//       并实现其特定的绘制（包括边框和填充）和交互逻辑。
// ---------------------------------------------------------------------------

#include "abstractshape.h" // 包含基类 AbstractShape 的头文件
#include <QRectF>          // 矩形的几何计算和绘制基于 QRectF 以获得更平滑的边缘和浮点精度

class RectangleShape : public AbstractShape // 继承自 AbstractShape
{
public:
    /// @brief RectangleShape 类的构造函数。
    /// @param topLeft 定义矩形的一个角点 (例如，用户鼠标按下的点)。
    /// @param bottomRight 定义矩形的另一个角点 (例如，用户鼠标释放的点)。
    /// @param borderColor 矩形的边框颜色，默认为黑色。
    /// @param penWidth 矩形的边框线宽，默认为1像素。
    /// @param filled 矩形是否被填充，默认为 false (不填充)。
    /// @param fillColor 矩形的填充颜色，默认为透明。
    RectangleShape(const QPoint &topLeft = QPoint(),
                   const QPoint &bottomRight = QPoint(),
                   const QColor &borderColor = Qt::black,
                   int penWidth = 1,
                   bool filled = false,
                   const QColor &fillColor = Qt::transparent);

    // --- 从 AbstractShape 继承并重写的虚函数 ---

    /// @brief 重写基类的 draw 方法，使用 QPainter 绘制矩形（可能带填充）。
    /// @param painter 指向 QPainter 对象的指针，矩形将在此 painter 上绘制。
    void draw(QPainter *painter) override;

    /// @brief 重写基类的 getBoundingRect 方法，计算并返回定义矩形的最小外接矩形。
    /// @return QRect 对象，表示矩形的包围盒。
    QRect getBoundingRect() const override;

    /// @brief 重写基类的 containsPoint 方法，判断给定点是否在矩形内部（如果填充）或其边框附近。
    /// @param point 要测试的点。
    /// @return 如果点在矩形上或其有效点击区域内，则返回 true；否则返回 false。
    bool containsPoint(const QPoint &point) const override;

    /// @brief 重写基类的 moveBy 方法，将定义矩形的两个对角点按给定的偏移量移动。
    /// @param offset QPoint 对象，表示在 x 和 y 方向上的移动量。
    void moveBy(const QPoint &offset) override;

    /// @brief 重写基类的 updateShape 方法，用于鼠标拖动创建矩形时更新其第二个定义点。
    /// @param point 当前鼠标指针的位置，通常用于更新 m_bottomRight (或 m_point2)。
    void updateShape(const QPoint &point) override;


    // --- RectangleShape 特有的公共方法 (可选的 Getters/Setters) ---

    /// @brief 获取定义矩形的第一个点 (例如左上角，取决于拖动方向)。
    QPoint getTopLeft() const { return m_topLeft; } // 或者叫 getPoint1()
    /// @brief 获取定义矩形的第二个点 (例如右下角，取决于拖动方向)。
    QPoint getBottomRight() const { return m_bottomRight; } // 或者叫 getPoint2()

    // void setTopLeft(const QPoint &point) { m_topLeft = point; }
    // void setBottomRight(const QPoint &point) { m_bottomRight = point; }

    /// @brief 获取一个标准化的 QRectF 对象，表示此矩形。
    /// 标准化确保其左上角坐标小于等于右下角坐标。
    QRectF getNormalizedRectF() const;


private:
    // 存储矩形特有的几何数据
    QPoint m_topLeft;     ///< 定义矩形的一个角点 (例如，鼠标按下的点)。
    QPoint m_bottomRight; ///< 定义矩形的另一个角点 (例如，鼠标释放的点或当前拖动到的点)。
};

#endif // RECTANGLESHAPE_H
