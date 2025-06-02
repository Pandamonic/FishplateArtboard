#ifndef ARTBOARDVIEW_H
#define ARTBOARDVIEW_H

// ---------------------------------------------------------------------------
// 描述: 定义 ArtboardView 类，这是程序的主要绘图区域。
//       它负责处理用户通过鼠标进行的绘图、选择、擦除等交互操作，
//       管理画布上的所有图形对象，以及实现撤销/重做功能。
// ---------------------------------------------------------------------------

#include <QWidget>
#include <QImage>
#include <QStack>
#include <QSet>
#include <QColor>
#include <QPoint>


#include "shared_types.h"     // 包含 ShapeType 枚举的定义
#include "abstractshape.h"    // 包含抽象图形基类 AbstractShape

// --- 向前声明 --- （因为有友元，所以向前声明）
// 向前声明具体的命令类和图形类，以减少头文件间的直接 #include 依赖，
// 仅在需要完整类定义时（通常在 .cpp 文件中）才包含其头文件。
class AbstractCommand;
class AddShapeCommand;
class DeleteShapeCommand;
class DeleteMultipleShapesCommand;
class ClearAllCommand;
class MoveShapeCommand;

/// @brief ArtboardView 类是用户进行绘图操作的主要画布区域。
///
/// 它负责以下核心功能：
/// - 接收和处理鼠标事件，以支持各种绘图工具（直线、矩形、自由曲线、椭圆、五角星等）的创建。
/// - 实现多种橡皮擦功能（点击删除、拖动删除、普通像素擦除）。
/// - 管理画布上所有已绘制的矢量图形对象 (存储在 shapesList 中)。
/// - 支持加载和显示背景图片，并能在背景图片上进行矢量绘图。
/// - 实现图形的选择和移动功能。
/// - 管理一个 Stack（栈），以支持无限级的撤销 (Undo) 和重做 (Redo) 操作。
/// - 将画布内容渲染到 QImage 以便导出为图片文件。
/// - 提供接口供 MainWindow 控制当前的绘图模式、颜色、线宽、填充等属性。
/// - 在命令栈状态改变时发射信号，以便更新UI（如撤销/重做按钮的启用状态）。
class ArtboardView : public QWidget
{
    Q_OBJECT
    // 声明所有需要访问 ArtboardView 私有成员的命令类为友元
    friend class AddShapeCommand;
    friend class DeleteShapeCommand;
    friend class DeleteMultipleShapesCommand;
    friend class ClearAllCommand;
    friend class MoveShapeCommand;

private:
    // --- 当前绘图属性 ---
    QColor currentDrawingColor;         ///< 当前绘图操作使用的边框颜色
    int currentPenWidth;                ///< 当前绘图操作使用的线宽
    QColor currentDrawingFillColor;     ///< 当前绘图操作使用的填充颜色 (主要用于闭合图形)
    bool currentIsFilled;               ///< 标记当前是否启用填充模式

    // --- 当前绘图/操作状态 ---
    ShapeType currentShapeType;           ///< 当前选中的绘图工具或操作模式 (来自 ShapeType 枚举)
    bool isCurrentlyDrawing;              ///< 标志位，表示用户是否正在进行鼠标拖动操作 (绘图、拖动擦除、拖动选择等)
    QPoint tempStartPoint;                ///< 在鼠标按下时，临时存储起始点坐标，用于创建新图形或计算位移
    AbstractShape* currentShapeInProgressPtr; ///< 指向当前正在通过鼠标拖动创建的图形对象 (在释放鼠标前)
    AbstractShape* m_selectedShape;         ///< 指向当前被“选择”工具选中的单个图形对象，若无则为 nullptr
    QPoint m_dragStartPoint_forCommand;   ///< 在使用“选择”工具拖动图形时，记录拖动开始的原始鼠标位置，用于MoveCommand

    // --- 数据存储 ---
    QImage m_backgroundImage;             ///< 加载的背景图片
    bool m_hasBackgroundImage;            ///< 标记当前是否存在有效的背景图片
    QVector<AbstractShape*> shapesList;   ///< 存储画布上所有已完成的、永久性的矢量图形对象的指针列表
    QSet<AbstractShape*> shapesToDeleteInCurrentDrag; ///< 用于“拖动删除笔画橡皮擦”操作时，临时存储在一次拖动中标记要删除的图形

    // --- 撤销/重做系统 ---
    QStack<AbstractCommand*> undoStack;   ///< 存储可撤销操作的命令栈
    QStack<AbstractCommand*> redoStack;   ///< 存储可重做操作的命令栈

    // --- 私有辅助方法 ---
    /// @brief 在“拖动删除笔画橡皮擦”模式下，检查给定点是否命中某个图形，并将其标记到待删除列表。
    void performStrokeEraseAtPoint(const QPoint &point);
    /// @brief 清空并释放 undoStack 和 redoStack 中的所有命令对象。
    void clearCommandStacks();
    /// @brief 清空 redoStack 并释放其中的命令对象，通常在新的可撤销操作执行后调用。
    void clearRedoStack();
    /// @brief 根据 undoStack 和 redoStack 的状态，发射 undoAvailabilityChanged 和 redoAvailabilityChanged 信号。
    void updateUndoRedoStatus();

public:
    /// @brief ArtboardView 类的构造函数。
    /// @param parent 父 QWidget 对象，默认为 nullptr。
    explicit ArtboardView(QWidget *parent = nullptr);

    /// @brief ArtboardView 类的析构函数。
    /// 负责释放在 shapesList 和命令栈中动态分配的图形对象和命令对象。
    ~ArtboardView() override;

    // --- 设置当前绘图模式和属性的公共接口 ---
    /// @brief 设置当前要绘制或操作的图形/工具类型。
    /// @param shape 新的图形/工具类型。
    void setCurrentShape(ShapeType shape);

    /// @brief 设置当前绘图的边框颜色。
    /// @param color 要设置的边框颜色。
    void setCurrentDrawingColor(const QColor &color);

    /// @brief 设置当前绘图的线宽。
    /// @param width 要设置的线宽。
    void setCurrentPenWidth(int width);

    /// @brief 设置当前绘图的填充颜色。
    /// @param color 要设置的填充颜色。
    void setCurrentDrawingFillColor(const QColor &color);

    /// @brief 设置当前是否启用填充模式。
    /// @param enable true 表示启用填充，false 表示不填充。
    void enableFill(bool enable);

    // --- 获取当前绘图属性的公共接口 ---
    /// @brief 获取当前设置的边框颜色。
    QColor getCurrentDrawingColor() const { return currentDrawingColor; }
    /// @brief 获取当前设置的线宽。
    int getCurrentPenWidth() const { return currentPenWidth; }
    /// @brief 获取当前设置的填充颜色。
    QColor getCurrentDrawingFillColor() const { return currentDrawingFillColor; }
    /// @brief 获取当前是否启用了填充模式。
    bool isFillEnabled() const { return currentIsFilled; }

    // --- 画布操作接口 ---
    /// @brief 清空画布上所有的矢量图形，并清空撤销/重做历史。背景图片不受影响。
    void clearAllShapes();
    /// @brief 设置画布的背景图片。
    /// @param image 要设置为背景的 QImage 对象。如果 image 为空，则清除背景。
    void setBackgroundImage(const QImage &image);
    /// @brief 清除当前画布的背景图片，恢复为默认背景。
    void clearBackgroundImage();
    /// @brief 将当前画布内容（包括背景图片和所有矢量图形）渲染到一个 QImage 对象中。
    /// @return 包含画布内容的 QImage 对象。
    QImage renderToImage();

    // --- 命令执行与撤销/重做接口 ---
    /// @brief 执行一个命令，将其压入撤销栈，并清空重做栈。
    /// @param command 指向要执行的 AbstractCommand 对象的指针。ArtboardView 将管理此命令对象的入栈。
    void executeCommand(AbstractCommand *command);
    /// @brief 执行撤销操作：从撤销栈弹出一个命令，执行其 undo() 方法，并将其压入重做栈。
    void undo();
    /// @brief 执行重做操作：从重做栈弹出一个命令，执行其 execute() 方法，并将其压入撤销栈。
    void redo();

signals:
    /// @brief 当撤销操作的可用性发生改变时发射此信号。
    /// @param available 如果为 true，表示当前有操作可以撤销；否则为 false。
    void undoAvailabilityChanged(bool available);

    /// @brief 当重做操作的可用性发生改变时发射此信号。
    /// @param available 如果为 true，表示当前有操作可以重做；否则为 false。
    void redoAvailabilityChanged(bool available);

protected:
    // --- 重写的 QWidget 事件处理函数 ---
    /// @brief QWidget 的绘图事件，负责绘制整个画布内容。
    void paintEvent(QPaintEvent *event) override;
    /// @brief QWidget 的鼠标按下事件，用于开始绘图、选择或擦除操作。
    void mousePressEvent(QMouseEvent *event) override;
    /// @brief QWidget 的鼠标移动事件，用于实时更新正在绘制的图形、拖动选中的图形或执行拖动擦除。
    void mouseMoveEvent(QMouseEvent *event) override;
    /// @brief QWidget 的鼠标释放事件，用于完成当前的绘图、选择或擦除操作，并可能创建命令对象。
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // ARTBOARDVIEW_H
