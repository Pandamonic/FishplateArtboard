#ifndef STARSHAPE_H
#define STARSHAPE_H

// ---------------------------------------------------------------------------
// 描述: 定义了五角星 (或其他多角星形) 图形类 StarShape，继承自 AbstractShape。
//       负责存储星形的几何数据（通过外接矩形定义）和角数，
//       并实现其特定的绘制和交互逻辑。
// ---------------------------------------------------------------------------

#include "abstractshape.h" // 包含基类 AbstractShape 的头文件
#include <QRectF>         // 星形的几何计算和绘制基于 QRectF
#include <QPolygonF>      // 用于存储和绘制星形的顶点

class StarShape : public AbstractShape // 继承自 AbstractShape
{
public:
    /// @brief StarShape 类的构造函数。
    /// @param point1 定义星形外接矩形的一个角点。
    /// @param point2 定义星形外接矩形的另一个角点。
    /// @param borderColor 星形的边框颜色，默认为黑色。
    /// @param penWidth 星形的边框线宽，默认为1像素。
    /// @param filled 星形是否被填充，默认为 false (不填充)。
    /// @param fillColor 星形的填充颜色，默认为透明。
    /// @param numPoints 星形的角数（例如，5 代表五角星），默认为5，必须大于2。
    StarShape(const QPoint &point1 = QPoint(),
              const QPoint &point2 = QPoint(),
              const QColor &borderColor = Qt::black,
              int penWidth = 1,
              bool filled = false,
              const QColor &fillColor = Qt::transparent,
              int numPoints = 5); // 新增角点数参数

    // --- 从 AbstractShape 继承并重写的虚函数 ---

    /// @brief 重写基类的 draw 方法，使用 QPainter 绘制星形。
    /// @param painter 指向 QPainter 对象的指针。
    void draw(QPainter *painter) override;

    /// @brief 重写基类的 getBoundingRect 方法，计算并返回星形的外接矩形。
    /// @return QRect 对象，表示星形的包围盒（即其定义的矩形区域）。
    QRect getBoundingRect() const override;

    /// @brief 重写基类的 containsPoint 方法，判断给定点是否在星形内部或其边框有效点击区域内。
    /// @param point 要测试的点。
    /// @return 如果点在星形上，则返回 true；否则返回 false。
    bool containsPoint(const QPoint &point) const override;

    /// @brief 重写基类的 moveBy 方法，将定义星形外接矩形的两个点按给定的偏移量移动。
    /// @param offset QPoint 对象，表示在 x 和 y 方向上的移动量。
    void moveBy(const QPoint &offset) override;

    /// @brief 重写基类的 updateShape 方法，用于鼠标拖动创建星形时更新其外接矩形的定义点。
    /// @param point 当前鼠标指针的位置，通常用于更新 m_point2。
    void updateShape(const QPoint &point) override;


    // --- StarShape 特有的公共方法 (可选的 Getters/Setters) ---

    /// @brief 获取定义星形外接矩形的第一个点。
    QPoint getPoint1() const { return m_point1; }
    /// @brief 获取定义星形外接矩形的第二个点。
    QPoint getPoint2() const { return m_point2; }
    /// @brief 获取星形的角数。
    int getNumPoints() const { return m_numPoints; }

    /// @brief 设置定义星形外接矩形的第一个点。
    // void setPoint1(const QPoint &point) { m_point1 = point; }
    /// @brief 设置定义星形外接矩形的第二个点。
    // void setPoint2(const QPoint &point) { m_point2 = point; }
    /// @brief 设置星形的角数。
    // void setNumPoints(int numPoints) { if (numPoints > 2) m_numPoints = numPoints; }


private:
    // 存储星形特有的几何数据
    QPoint m_point1;    ///< 定义星形外接矩形的一个角点
    QPoint m_point2;    ///< 定义星形外接矩形的另一个角点
    int m_numPoints;    ///< 星形的角数 (例如 5 代表标准的五角星)

    // 私有辅助函数
    /// @brief 根据 m_point1 和 m_point2 计算并返回标准化的浮点型外接矩形。
    QRectF getNormalizedRectF() const;
    /// @brief 根据给定的外接矩形 bounds 和角数 m_numPoints，计算并返回星形的所有顶点。
    /// @param bounds 定义星形绘制范围的 QRectF 对象。
    /// @return QPolygonF 对象，包含星形的所有顶点坐标。
    QPolygonF calculateStarVertices(const QRectF &bounds) const;
};

#endif // STARSHAPE_H
