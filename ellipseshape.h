#ifndef ELLIPSESHAPE_H
#define ELLIPSESHAPE_H

// ---------------------------------------------------------------------------
// 描述: 定义了椭圆图形类 EllipseShape，继承自 AbstractShape。
//       负责存储椭圆的几何数据（通过其外接矩形的两个对角点定义）
//       并实现其特定的绘制（包括边框和填充）和交互逻辑。
// ---------------------------------------------------------------------------

#include "abstractshape.h" // 包含基类 AbstractShape 的头文件
#include <QRectF>         // 椭圆的几何计算和绘制通常基于 QRectF 以获得更平滑的边缘

class EllipseShape : public AbstractShape // 继承自 AbstractShape
{
public:
    /// @brief EllipseShape 类的构造函数。
    /// @param point1 定义椭圆外接矩形的一个角点。
    /// @param point2 定义椭圆外接矩形的另一个角点。
    /// @param borderColor 椭圆的边框颜色，默认为黑色。
    /// @param penWidth 椭圆的边框线宽，默认为1像素。
    /// @param filled 椭圆是否被填充，默认为 false (不填充)。
    /// @param fillColor 椭圆的填充颜色，默认为透明。
    EllipseShape(const QPoint &point1 = QPoint(),
                 const QPoint &point2 = QPoint(),
                 const QColor &borderColor = Qt::black,
                 int penWidth = 1,
                 bool filled = false,
                 const QColor &fillColor = Qt::transparent);

    // --- 从 AbstractShape 继承并重写的虚函数 ---

    /// @brief 重写基类的 draw 方法，使用 QPainter 绘制椭圆（可能带填充）。
    /// @param painter 指向 QPainter 对象的指针，椭圆将在此 painter 上绘制。
    void draw(QPainter *painter) override;

    /// @brief 重写基类的 getBoundingRect 方法，计算并返回定义椭圆的最小外接矩形。
    /// @return QRect 对象，表示椭圆的包围盒。
    QRect getBoundingRect() const override;

    /// @brief 重写基类的 containsPoint 方法，判断给定点是否在椭圆内部（如果填充）或其边框附近。
    /// @param point 要测试的点。
    /// @return 如果点在椭圆上或其有效点击区域内，则返回 true；否则返回 false。
    bool containsPoint(const QPoint &point) const override;

    /// @brief 重写基类的 moveBy 方法，将定义椭圆外接矩形的两个点按给定的偏移量移动。
    /// @param offset QPoint 对象，表示在 x 和 y 方向上的移动量。
    void moveBy(const QPoint &offset) override;

    /// @brief 重写基类的 updateShape 方法，用于鼠标拖动创建椭圆时更新其外接矩形的定义点。
    /// @param point 当前鼠标指针的位置，通常用于更新 m_point2。
    void updateShape(const QPoint &point) override;


    // --- EllipseShape 特有的公共方法 (可选的 Getters) ---

    /// @brief 获取定义椭圆外接矩形的第一个点。
    QPoint getPoint1() const { return m_point1; }
    /// @brief 获取定义椭圆外接矩形的第二个点。
    QPoint getPoint2() const { return m_point2; }

private:
    // 存储椭圆特有的几何数据
    QPoint m_point1;    ///< 定义椭圆外接矩形的一个角点。
    QPoint m_point2;    ///< 定义椭圆外接矩形的另一个角点。

    // 私有辅助函数
    /// @brief 根据 m_point1 和 m_point2 计算并返回一个标准化的 QRectF 对象。
    /// 标准化确保矩形的左上角坐标小于等于右下角坐标。
    QRectF getNormalizedRectF() const;
};

#endif // ELLIPSESHAPE_H
