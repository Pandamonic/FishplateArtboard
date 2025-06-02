#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

/// @brief 定义了画布上可以绘制或操作的图形/工具类型
enum ShapeType {
    None,                   ///< 无操作或选择了“选择”工具
    Line,                   ///< 直线工具
    Rectangle,              ///< 矩形工具
    Freehand,               ///< 自由画笔工具
    StrokeEraser,           ///< 点击式笔画橡皮擦
    DraggingStrokeEraser,   ///< 拖动式笔画橡皮擦
    NormalEraser,           ///< 普通橡皮擦 (用背景色绘制，暂时确实缺少精力做消除笔迹的橡皮擦)
    Ellipse,                ///< 椭圆工具
    Star                    ///< 五角星工具
};

#endif // SHARED_TYPES_H
