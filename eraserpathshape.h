#ifndef ERASERPATHSHAPE_H
#define ERASERPATHSHAPE_H

// ---------------------------------------------------------------------------
// 描述: 定义了普通橡皮擦路径图形类 EraserPathShape，继承自 AbstractShape。
//       它通过存储一个点的序列 (QVector<QPoint>) 来表示用户的擦除轨迹，
//       并在绘制时使用画布的背景色覆盖下方内容，从而实现“擦除”效果。
//       其内部实现与 FreehandPathShape 非常相似。
// ---------------------------------------------------------------------------

#include "abstractshape.h" // 包含基类 AbstractShape 的头文件
#include <QVector>        // 用于存储构成路径的点集
#include <QPoint>         // 点集中的元素是 QPoint 类型
#include <QPainterPath>   // 使用 QPainterPath 来构建和绘制路径

class EraserPathShape : public AbstractShape // 继承自 AbstractShape
{
public:
    /// @brief EraserPathShape 类的构造函数。
    /// @param points (可选) 构成橡皮擦轨迹的初始点集。默认为空 QVector。
    /// @param eraserWidth 橡皮擦的宽度（即路径的线宽），默认为2像素。
    /// @param eraserColor 橡皮擦用于绘制的颜色，通常是画布的背景色 (例如 Qt::white)。
    ///                    这个颜色会传递给基类的 shapeColor。
    EraserPathShape(const QVector<QPoint> &points = QVector<QPoint>(),
                    int eraserWidth = 2,
                    const QColor &eraserColor = Qt::white); // 橡皮擦颜色默认为白色

    // --- 从 AbstractShape 继承并重写的虚函数 ---

    /// @brief 重写基类的 draw 方法，使用 QPainter 以指定的橡皮擦颜色和宽度绘制路径。
    /// @param painter 指向 QPainter 对象的指针，路径将在此 painter 上绘制。
    void draw(QPainter *painter) override;

    /// @brief 重写基类的 getBoundingRect 方法，计算并返回橡皮擦路径的最小外接矩形。
    /// @return QRect 对象，表示路径的包围盒（已考虑橡皮擦宽度）。
    QRect getBoundingRect() const override;

    /// @brief 重写基类的 containsPoint 方法。
    /// 对于橡皮擦路径本身是否“可被选中”（例如被笔画橡皮擦删除），
    /// 此方法判断给定点是否在其有效点击区域内（考虑橡皮擦宽度）。
    /// @param point 要测试的点。
    /// @return 如果点在路径上或其描边区域内，则返回 true；否则返回 false。
    bool containsPoint(const QPoint &point) const override;

    /// @brief 重写基类的 moveBy 方法，将橡皮擦路径中的所有点按给定的偏移量移动。
    /// （如果橡皮擦痕迹被设计为可移动的话）。
    /// @param offset QPoint 对象，表示在 x 和 y 方向上的移动量。
    void moveBy(const QPoint &offset) override;

    /// @brief 重写基类的 updateShape 方法，用于鼠标拖动绘制橡皮擦路径时向路径中添加新的点。
    /// @param point 当前鼠标指针的位置，将作为新点添加到路径中。
    void updateShape(const QPoint &point) override;


    // --- EraserPathShape 特有的公共方法 (与 FreehandPathShape 类似) ---

    /// @brief 获取构成此橡皮擦路径的所有点的列表 (const引用，用于读取)。
    const QVector<QPoint>& getPoints() const { return m_points; }

    /// @brief 设置构成此橡皮擦路径的点集。
    /// 调用此方法后，会重新构建内部的 QPainterPath。
    /// @param points 新的点集。
    void setPoints(const QVector<QPoint> &points);

    /// @brief 向此橡皮擦路径的末尾添加一个新点。
    /// 调用此方法后，会更新内部的 QPainterPath。
    /// @param point 要添加的 QPoint。
    void addPoint(const QPoint &point);


private:
    QVector<QPoint> m_points;   ///< 存储构成橡皮擦轨迹的所有 QPoint 点的序列。
    QPainterPath m_painterPath; ///< 根据 m_points 构建的 QPainterPath 对象，用于高效绘制和精确计算。

    /// @brief 私有辅助函数，根据当前的 m_points 点集更新（重建）内部的 m_painterPath 对象。
    /// 当 m_points 发生变化时应调用此方法。
    void buildPath();
};

#endif // ERASERPATHSHAPE_H
