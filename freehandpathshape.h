#ifndef FREEHANDPATHSHAPE_H
#define FREEHANDPATHSHAPE_H

// ---------------------------------------------------------------------------
// 描述: 定义了自由画笔路径图形类 FreehandPathShape，继承自 AbstractShape。
//       它通过存储一个点的序列 (QVector<QPoint>) 来表示用户绘制的自由曲线，
//       并使用 QPainterPath 进行高效绘制和精确的点击检测。
// ---------------------------------------------------------------------------

#include "abstractshape.h" // 包含基类 AbstractShape 的头文件
#include <QVector>        // 用于存储构成路径的点集
#include <QPoint>         // 点集中的元素是 QPoint 类型
#include <QPainterPath>   // 使用 QPainterPath 来构建和绘制路径，可以获得较好的效果

class FreehandPathShape : public AbstractShape // 继承自 AbstractShape
{
public:
    /// @brief FreehandPathShape 类的构造函数。
    /// @param points (可选) 构成自由曲线的初始点集。默认为空 QSet。
    /// @param borderColor 路径的颜色，默认为黑色。
    /// @param penWidth 路径的线宽，默认为1像素。
    /// @note 自由曲线通常不进行填充，因此基类的 filled 和 fillColor 参数会使用默认值 (false, Qt::transparent)。
    FreehandPathShape(const QVector<QPoint> &points = QVector<QPoint>(),
                      const QColor &borderColor = Qt::black,
                      int penWidth = 1);
    // 构造函数只接收其核心属性，填充相关的由基类构造函数硬编码

    // --- 从 AbstractShape 继承并重写的虚函数 ---

    /// @brief 重写基类的 draw 方法，使用 QPainter 绘制自由曲线路径。
    /// @param painter 指向 QPainter 对象的指针，路径将在此 painter 上绘制。
    void draw(QPainter *painter) override;

    /// @brief 重写基类的 getBoundingRect 方法，计算并返回自由曲线路径的最小外接矩形。
    /// @return QRect 对象，表示路径的包围盒。
    QRect getBoundingRect() const override;

    /// @brief 重写基类的 containsPoint 方法，判断给定点是否在自由曲线路径的有效点击区域内（考虑线宽）。
    /// @param point 要测试的点。
    /// @return 如果点在路径上或其描边区域内，则返回 true；否则返回 false。
    bool containsPoint(const QPoint &point) const override;

    /// @brief 重写基类的 moveBy 方法，将自由曲线路径中的所有点按给定的偏移量移动。
    /// @param offset QPoint 对象，表示在 x 和 y 方向上的移动量。
    void moveBy(const QPoint &offset) override;

    /// @brief 重写基类的 updateShape 方法，用于鼠标拖动绘制自由曲线时向路径中添加新的点。
    /// @param point 当前鼠标指针的位置，将作为新点添加到路径中。
    void updateShape(const QPoint &point) override;


    // --- FreehandPathShape 特有的公共方法 ---

    /// @brief 获取构成此自由曲线的所有点的列表 (const引用，用于读取)。
    /// @return 包含所有 QPoint 的 QVector 的 const 引用。
    const QVector<QPoint>& getPoints() const { return m_points; }

    /// @brief 设置构成此自由曲线的点集。
    /// 调用此方法后，会重新构建内部的 QPainterPath。
    /// @param points 新的点集。
    void setPoints(const QVector<QPoint> &points);

    /// @brief向此自由曲线的末尾添加一个新点。
    /// 调用此方法后，会更新内部的 QPainterPath。
    /// @param point 要添加的 QPoint。
    void addPoint(const QPoint &point);

private:
    QVector<QPoint> m_points;   ///< 存储构成自由曲线的所有 QPoint 点的序列。
    QPainterPath m_painterPath; ///< 根据 m_points 构建的 QPainterPath 对象，用于高效绘制和精确计算。

    /// @brief 私有辅助函数，根据当前的 m_points 点集更新（重建）内部的 m_painterPath 对象。
    /// 当 m_points 发生变化时（如添加点、设置新点集、移动所有点），应调用此方法。
    void buildPath();
};

#endif // FREEHANDPATHSHAPE_H
