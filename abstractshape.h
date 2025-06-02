#ifndef ABSTRACTSHAPE_H
#define ABSTRACTSHAPE_H

// ---------------------------------------------------------------------------
// 描述: 定义了所有可绘制图形的抽象基类 AbstractShape。
//      这个类提供了图形对象共有的属性和必须实现的接口（如绘制、获取边界等）。
// ---------------------------------------------------------------------------

#include <QPainter>     // 用于绘图操作的 QPainter
#include <QColor>       // 用于处理颜色属性
#include <QPoint>       // 用于确定坐标点
#include <QRect>        // 用于边界矩形
#include <QVector>      // (如果基类或派生类需要点集，例如在 updateShape 中)
#include "shared_types.h" // 包含 ShapeType 枚举的定义

/// @brief AbstractShape 是所有具体图形（如直线、矩形、椭圆等）的抽象基类。
/// 它定义了图形对象共有的属性（如边框颜色、线宽、填充状态、填充颜色）
/// 以及必须由派生类实现的纯虚函数（如 draw, getBoundingRect, containsPoint, moveBy）
/// 和一些通用的虚函数（如 updateShape）。
/// 这个类不能被直接实例化。
class AbstractShape
{
public:
    /// @brief AbstractShape 类的构造函数。
    /// @param type 图形的类型 (来自 ShapeType 枚举)。
    /// @param borderColor 图形的边框颜色，默认为黑色。
    /// @param penWidth 图形的边框线宽，默认为1像素。
    /// @param filled 图形是否被填充，默认为 false (不填充)。
    /// @param fillColor 图形的填充颜色，默认为透明。
    AbstractShape(ShapeType type = ShapeType::None,
                  const QColor &borderColor = Qt::black,
                  int penWidth = 1,
                  bool filled = false,
                  const QColor &fillColor = Qt::transparent)
        : shapeType(type),
        shapeColor(borderColor),    // shapeColor在这里代表边框颜色
        shapePenWidth(penWidth),
        m_isFilled(filled),
        m_shapeFillColor(fillColor)
    {
        // 构造函数体为空，这个程序中属性将通过初始化列表设置。
    }

    /// @brief 虚析构函数。
    /// 对于可能通过基类指针删除派生类对象的类，确保派生类的析构函数能被正确调用，防止资源泄漏。
    virtual ~AbstractShape() {}

    // --- 核心绘图与交互接口 (纯虚函数，必须由派生类实现) ---

    /// @brief 纯虚函数，用于在给定的 QPainter上绘制图形。
    /// @param painter 指向 QPainter对象的指针，图形将在此painter上绘制。
    virtual void draw(QPainter *painter) = 0;

    /// @brief 纯虚函数，用于获取图形的最小外接矩形 (也就是我的Surrounding box)。
    /// @return 返回一个 QRect对象，表示图形的边界。
    virtual QRect getBoundingRect() const = 0;

    /// @brief 纯虚函数，用于判断给定的点是否在该图形上或其有效点击区域内。
    /// @param point 要测试的点。
    /// @return 如果点在图形上，则返回 true；否则返回 false。
    virtual bool containsPoint(const QPoint &point) const = 0;

    /// @brief 纯虚函数，用于将图形按给定的偏移量移动。
    /// @param offset 一个 QPoint 对象，表示在 x 和 y 方向上的移动量。
    virtual void moveBy(const QPoint &offset) = 0;

    // --- 用于鼠标拖动创建/修改图形时的更新接口 (虚函数) ---

    /// @brief 虚函数，用于在鼠标拖动过程中更新图形的几何形状，根据新的鼠标点来调整图形的某个定义点或尺寸。
    /// @param point 当前鼠标指针的位置。
    virtual void updateShape(const QPoint &point) { Q_UNUSED(point); }


    // --- 公共的 Getter 和 Setter 方法 ---

    /// @brief 获取图形的类型。
    /// @return 图形的 ShapeType 枚举值。
    ShapeType getType() const { return shapeType; }

    /// @brief 获取图形的边框颜色。
    /// @return QColor 对象，表示边框颜色。
    QColor getBorderColor() const { return shapeColor; }

    /// @brief 获取图形的边框线宽。
    /// @return int 值，表示线宽（像素）。
    int getPenWidth() const { return shapePenWidth; }

    /// @brief 判断图形是否被填充。
    /// @return 如果图形被填充，则返回 true；否则返回 false。
    bool isFilled() const { return m_isFilled; }

    /// @brief 获取图形的填充颜色。
    /// @return QColor 对象，表示填充颜色。
    QColor getFillColor() const { return m_shapeFillColor; }

    /// @brief 设置图形的边框颜色。
    /// @param color 新的边框颜色。
    void setBorderColor(const QColor &color) { shapeColor = color; }

    /// @brief 设置图形的边框线宽。
    /// @param width 新的线宽，必须大于0。
    void setPenWidth(int width) { if (width > 0) shapePenWidth = width; }

    /// @brief 设置图形是否被填充。
    /// @param filled true 表示填充，false 表示不填充。
    void setFilled(bool filled) { m_isFilled = filled; }

    /// @brief 设置图形的填充颜色。
    /// @param color 新的填充颜色。
    void setFillColor(const QColor &color) { m_shapeFillColor = color; }

protected:
    // 保护成员变量，允许派生类直接访问以进行绘制和几何计算。
    ShapeType shapeType;        ///图形的具体类型 (Line, Rectangle 等)
    QColor shapeColor;          ///图形的边框颜色 (原 color 成员)
    int shapePenWidth;          ///图形的边框线宽
    bool m_isFilled;            ///标记图形是否应该被填充
    QColor m_shapeFillColor;    ///图形的填充颜色
};

#endif // ABSTRACTSHAPE_H
