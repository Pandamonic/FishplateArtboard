// ---------------------------------------------------------------------------
// 描述: ArtboardView 类的实现文件。
//       包含了所有绘图、鼠标交互、图形管理、命令处理等核心逻辑。
// ---------------------------------------------------------------------------

#include "artboardview.h"
#include <QDebug>
#include <QPainter>         // paintEvent 和 renderToImage 需要
#include <QMouseEvent>      // 鼠标事件处理函数需要
#include <QPainterPath>     // 一些图形的绘制或 containsPoint 可能用到
#include <QPainterPathStroker> // containsPoint 的精确判断可能用到
#include <QImage>           // renderToImage 和背景图片处理需要
#include <QPalette>         // 用于获取和设置背景色
#include <algorithm>        // 用于 sort (在 mouseReleaseEvent 的拖动删除中)

// --- 包含所有具体的图形类和命令类的头文件 ---
#include "lineshape.h"
#include "rectangleshape.h"
#include "freehandpathshape.h"
#include "ellipseshape.h"
#include "starshape.h"
#include "eraserpathshape.h"

#include "abstractcommand.h"
#include "addshapecommand.h"
#include "deleteshapecommand.h"
#include "deletemultipleshapescommand.h"
#include "clearallcommand.h"
#include "moveshapecommand.h"

/// @brief ArtboardView 类的构造函数。
/// @param parent 指向父 QWidget 对象的指针，默认为 nullptr。
/// 主要功能：
/// - 初始化所有成员变量为其默认状态（例如，当前颜色、线宽、工具类型、各种列表和标志位）。
/// - 设置画布的基本外观属性，如自动填充背景和默认背景色（白色）。
/// - 发射初始的撤销/重做可用性信号，以确保相关UI按钮处于正确的初始禁用状态。
ArtboardView::ArtboardView(QWidget *parent)
    : QWidget{parent}, // 调用基类构造函数
    // --- 初始化当前绘图属性 ---
    currentDrawingColor(Qt::black),           // 默认边框（画笔）颜色为黑色
    currentPenWidth(2),                       // 默认线宽为2像素
    currentDrawingFillColor(Qt::transparent), // 默认填充颜色为透明
    currentIsFilled(false),                   // 默认不启用填充模式
    // --- 初始化当前操作状态和数据 ---
    currentShapeType(ShapeType::None),        // 初始工具模式为“无”或“选择”
    isCurrentlyDrawing(false),                // 初始状态下，用户没有正在进行绘图或拖动操作
    currentShapeInProgressPtr(nullptr),       // 初始没有正在绘制过程中的图形对象
    m_selectedShape(nullptr),                 // 初始没有被选中的图形对象
    m_dragStartPoint_forCommand(0,0),         // (QPoint 默认构造为 0,0) 用于记录拖动命令的起始点
    // --- 初始化背景图片状态 ---
    m_backgroundImage(),                      // m_backgroundImage (QImage) 被默认构造为空图像
    m_hasBackgroundImage(false)               // 初始标记为没有背景图片
// undoStack, redoStack (QStack) 会被默认构造为空栈
// shapesList (QVector) 会被默认构造为空列表
// shapesToDeleteInCurrentDrag (QSet) 会被默认构造为空集合
// tempStartPoint (QPoint) 会被默认构造
{
    // 1. 设置画布可以自动填充其背景。
    //    当 paintEvent 被调用时，如果 autoFillBackground 为 true，
    //    QWidget 会在调用我们自己实现的 paintEvent 内容之前，先用 palette 的 Window 角色颜色填充背景。
    setAutoFillBackground(true);

    // 2. 获取当前控件的调色板，并设置窗口背景色 (QPalette::Window) 为白色。
    //    这确保了我们的画布有一个明确的、统一的背景颜色。
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal); // 应用修改后的调色板

    // 3. 发射初始的撤销/重做可用性信号。
    //    在程序启动时，撤销栈和重做栈都是空的，所以这两个操作都不可用。
    emit undoAvailabilityChanged(false);
    emit redoAvailabilityChanged(false);
}

/// @brief ArtboardView 类的析构函数。
/// 负责在 ArtboardView 对象被销毁时，清理所有动态分配的资源，以防止内存泄漏。
ArtboardView::~ArtboardView()
{
    // 调用 clearAllShapes() 会负责删除 shapesList 中的图形对象，并且它内部也会调用 clearCommandStacks()。
    clearAllShapes();
    qDebug() << "ArtboardView instance destroyed.";
}

/// @brief 清空画布上所有已绘制的矢量图形，并清除相关的撤销/重做历史。
/// 此操作本身通常应该被封装为一个可撤销的命令 (ClearAllCommand)，
/// 而不是直接修改内部状态后简单地调用此方法。
/// 不过，作为内部清理方法，它也需要在析构时被调用。
void ArtboardView::clearAllShapes()
{
    qDebug() << "ArtboardView::clearAllShapes() - Clearing all vector shapes and command history.";

    // 1. 删除并释放 shapesList 中所有 AbstractShape 对象所占用的内存。
    //    查阅一些资料，我发现 qDeleteAll 是 Qt 提供的便捷函数，它会遍历容器中的每个指针并对其调用 delete。
    qDeleteAll(shapesList);
    // 2. 从 QVector 中移除所有指针，使其变为空列表。
    shapesList.clear();

    // 3. 清空并释放 undo 和 redo 命令栈中的所有 AbstractCommand 对象。
    clearCommandStacks(); // 这个函数会负责 delete 命令对象，并更新按钮状态

    // 4. 重置选中状态和可能正在绘制的图形
    if (m_selectedShape) {
        m_selectedShape = nullptr;
    }
    if (currentShapeInProgressPtr) {
        delete currentShapeInProgressPtr; // 如果有未完成的图形，也一并删除
        currentShapeInProgressPtr = nullptr;
    }
    isCurrentlyDrawing = false; // 重置绘图状态

    // 5. 请求重绘界面，显示一个空白的画布（或仅有背景图片）。
    update();

    // updateUndoRedoStatus() 应该在 clearCommandStacks() 内部被调用，所以这里不需要重复。
    qDebug() << "ArtboardView: All vector shapes and command history have been cleared.";
}

/// @brief 设置 ArtboardView 当前激活的绘图工具或操作模式。
/// @param shape 要设置的新的 ShapeType。
///
/// 当用户从 MainWindow 的工具栏选择不同的工具时，此方法被调用。
/// 它会更新内部的 currentShapeType 状态。
/// 如果新的工具不是“选择”工具 (ShapeType::None)，并且之前有图形被选中，
/// 此方法还会清除当前的选中状态 (将 m_selectedShape 设为 nullptr) 并更新视图。
void ArtboardView::setCurrentShape(ShapeType shape)
{
    if (currentShapeType != shape) { // 仅当工具类型实际改变时才执行操作
        ShapeType oldShapeType = currentShapeType; // 记录旧的工具类型，方便调试
        currentShapeType = shape;                  // 更新当前工具类型
        qDebug() << "ArtboardView: Current shape/tool set from" << oldShapeType << "to" << currentShapeType;

        // 如果切换到的新工具不是“选择”工具 (ShapeType::None)，
        // 那么任何之前可能存在的图形选中状态都应该被清除。
        if (currentShapeType != ShapeType::None) {
            if (m_selectedShape) { // 检查是否当前有图形被选中
                m_selectedShape = nullptr; // 清除选中状态
                qDebug() << "ArtboardView: Selection cleared because a new drawing/eraser tool was selected.";
                update(); // 请求重绘，以移除界面上可能存在的旧的选中指示框
            }
        }
        // 如果新选择的工具是“选择”工具 (ShapeType::None)，则不在此处清除选中，
        // 用户可能希望继续操作已选中的图形。清除选中的逻辑（例如点击空白区域）
        // 会在 mousePressEvent 中针对 ShapeType::None 模式进行处理。
    }
}

/// @brief QWidget 的核心绘图事件处理函数，在每次需要重绘 ArtboardView 时被调用。
///
/// 负责按照正确的层次顺序绘制画布上的所有可见元素：
/// 1. 如果 QWidget::paintEvent 和 palette 未能完全覆盖，绘制默认画布背景色。
/// 2. 绘制用户加载的背景图片 (m_backgroundImage)，如果存在且有效。
/// 3. 遍历并绘制 shapesList 中的所有已完成的矢量图形对象（通过多态调用其 draw() 方法）。
/// 4. 如果当前有图形被选中 (m_selectedShape 不为 nullptr)，则绘制选中指示框。
/// 5. 如果用户当前正在使用绘图工具创建新图形 (isCurrentlyDrawing 为 true,
///    currentShapeInProgressPtr 有效, 且非选择模式，则绘制该图形的实时预览。
/// @param event 指向 QPaintEvent 对象的指针，包含了绘图事件的相关信息 (通常不直接使用)。
void ArtboardView::paintEvent(QPaintEvent *event)
{
    // 1. 调用基类 QWidget 的 paintEvent 实现。
    //    这有助于处理一些 Qt 内部的绘制需求，例如，如果 autoFillBackground 为 true，
    //    它会使用当前 palette 的 QPalette::Window 角色颜色来填充背景。
    QWidget::paintEvent(event);

    // 2. 创建 QPainter 对象，并设置其绘图目标为当前的 ArtboardView 实例 (this)。
    //    QPainter 是 Qt 中进行所有2D绘图操作的核心类。
    //    在栈上创建 QPainter 对象，当函数结束时它会自动销毁并调用 painter.end()。
    QPainter painter(this);

    // 3. 抗锯齿渲染提示，使绘制的图形边缘更平滑。
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 4. 绘制背景图片 (如果用户已加载)
    if (m_hasBackgroundImage && !m_backgroundImage.isNull()) {
        // 获取当前 ArtboardView 的绘图区域矩形
        QRectF targetRect = this->rect();
        // 获取背景图片的原始矩形 (用于计算缩放比例)
        // QRectF sourceRect = m_backgroundImage.rect(); // 这行其实没直接用到，可以省略

        // 将背景图片按保持自身宽高比的方式进行缩放，使其能适应 targetRect 的大小。
        // Qt::KeepAspectRatio: 保持宽高比，可能会有留白。
        // Qt::SmoothTransformation: 使用高质量的平滑缩放算法。
        QImage scaledImage = m_backgroundImage.scaled(targetRect.size().toSize(),
                                                      Qt::KeepAspectRatio,
                                                      Qt::SmoothTransformation);
        // 计算图片在 targetRect 中居中显示时的左上角绘制坐标
        qreal x = targetRect.left() + (targetRect.width() - scaledImage.width()) / 2.0;
        qreal y = targetRect.top() + (targetRect.height() - scaledImage.height()) / 2.0;

        // 在计算好的位置绘制缩放后的背景图片
        painter.drawImage(QPointF(x, y), scaledImage);
    }
    // 如果没有背景图片 (m_hasBackgroundImage 为 false)，并且 autoFillBackground 为 true，
    // QWidget::paintEvent(event) 应该已经用 palette().window().color() 填充了背景。
    // 如果需要更明确的控制，可以在这里的 else 分支中用 painter.fillRect() 填充默认背景色。

    // 5. 绘制所有已经完成并存储在 shapesList 中的图形对象
    for (AbstractShape *shape : shapesList) { // 使用范围for循环遍历指针列表
        if (shape) { // 确保指针有效，避免操作空悬指针
            // 通过基类指针调用 draw() 方法。由于 draw() 是虚函数，
            // 这里会发生多态：实际调用的是 shape 指针所指向的具体派生类对象
            // (LineShape, RectangleShape 等) 中重写的 draw() 方法。
            shape->draw(&painter);
        }
    }

    // 6. 如果当前有图形被选中 (m_selectedShape 不为 nullptr)，则为其绘制选中指示框
    if (m_selectedShape) {
        QRect boundingBox = m_selectedShape->getBoundingRect(); // 获取选中图形的包围盒
        // 检查包围盒是否有效 (非空且宽度高度为正)
        if (!boundingBox.isNull() && boundingBox.isValid()) {
            QPen selectionPen; // 创建一个新的画笔用于绘制选中框
            selectionPen.setColor(Qt::blue);         // 设置选中框颜色为蓝色
            selectionPen.setStyle(Qt::DashLine);    // 设置选中框样式为虚线
            selectionPen.setWidth(1);             // 设置选中框线宽为1像素
            painter.setPen(selectionPen);           // 将选中框画笔应用到 painter

            // 确保选中框不被意外填充，将其画刷设置为空/透明
            painter.setBrush(Qt::NoBrush);

            // 绘制选中框。使用 adjusted() 可以让选中框比实际包围盒稍大一些，
            // 以便更清晰地指示选中状态，避免与图形本身边框完全重合。
            // adjusted(-left, -top, right, bottom) 分别向外扩展。
            painter.drawRect(boundingBox.adjusted(-3, -3, 3, 3));

            // (未来展望：可以在这里根据包围盒的角点和边中点绘制控制手柄，用于缩放和旋转)
        }
    }

    // 7. 如果用户当前正在使用某个绘图工具进行绘制 (且不是选择工具)，
    //    则绘制 currentShapeInProgressPtr 指向的那个正在形成的图形的实时预览。
    if (isCurrentlyDrawing && currentShapeInProgressPtr && currentShapeType != ShapeType::None) {
        // currentShapeInProgressPtr 指向的是一个具体的图形对象 (如 LineShape 实例)，
        // 它在被创建时就已经被赋予了当前的绘图属性（颜色、线宽、填充状态等）。
        // 调用它的 draw() 方法，它会使用自身的属性和几何数据进行绘制。
        currentShapeInProgressPtr->draw(&painter); // 多态调用
    }
}

/// @brief 私有辅助函数，在“拖动删除笔画橡皮擦”模式下被调用。
///
/// 当鼠标按住并拖动时，此函数会检查当前鼠标指针下的点 (`point`)
/// 是否命中了画布上的任何已绘制图形。如果命中，并且该图形尚未被标记为
/// 在本次拖动操作中待删除，则将其添加到 `shapesToDeleteInCurrentDrag` 集合中。
/// 实际的删除操作将在鼠标释放时（`mouseReleaseEvent`）根据此集合统一处理，
/// 以避免在遍历 `shapesList` 的同时修改它可能导致的迭代器失效或逻辑错误。
///
/// @param point 当前鼠标指针在 ArtboardView 坐标系下的位置。
void ArtboardView::performStrokeEraseAtPoint(const QPoint &point)
{
    // 遍历当前画布上所有已绘制的图形对象
    for (AbstractShape* shape : shapesList) {
        // 安全检查：确保图形对象指针有效，并且调用其 containsPoint 方法判断当前鼠标点是否在该图形的有效点击区域内。
        // containsPoint 方法由每个具体的图形派生类（LineShape, RectangleShape 等）实现。
        if (shape && shape->containsPoint(point)) {
            // 如果点中了某个图形，我们需要检查这个图形是否已经被添加到本次拖动操作的待删除集合中。
            // shapesToDeleteInCurrentDrag 是一个 QSet，可以高效地检查元素是否存在。
            if (!shapesToDeleteInCurrentDrag.contains(shape)) { // 如果这个图形还没有被标记过
                // 将被命中的图形对象指针添加到待删除集合中。
                shapesToDeleteInCurrentDrag.insert(shape);
                // 输出调试信息，记录哪个图形在哪个点被标记了。
                qDebug() << "ArtboardView: Shape at address" << (void*)shape
                         << "of type" << shape->getType()
                         << "marked for deletion by dragging eraser at point:" << point;
            }
            // 如果一个点可能同时命中多个重叠的图形，而我们只想标记最上面的那个，
            // 那么在找到第一个命中的图形并将其加入待删除集合后，可以考虑 break 掉这个 for 循环。
            // 但对于“拖动删除”橡皮擦，通常的行为是路径上所有碰到的图形都会被标记。
            // 我们当前的遍历顺序（从列表头到尾，即从底层到顶层图形）和不 break 的做法，
            // 意味着如果鼠标同时覆盖多个图形，它们都会被标记。
            // 如果希望只标记最顶层的，遍历 shapesList 时应该从后往前。
            // 不过，performStrokeEraseAtPoint 的调用是在 mouseMoveEvent 中，
            // 鼠标的每个小段移动都会触发一次，所以即使有重叠，只要鼠标划过，它们都会被逐个检测到。
        }
    }
}

/// 这个是最重要的函数之一
/// @brief 处理鼠标在画布上按下的事件。
///
/// 此函数是所有用户交互（绘图、选择、擦除）的起点。
/// 它根据当前 ArtboardView 的状态 (currentShapeType) 执行不同的操作：
/// - ShapeType::None (选择工具): 尝试选中鼠标点击位置的图形，并为可能的拖动操作做准备。
/// - ShapeType::StrokeEraser (点击式笔画橡皮擦): 查找并删除鼠标点击位置的单个图形（通过 DeleteShapeCommand）。
/// - ShapeType::DraggingStrokeEraser (拖动式笔画橡皮擦): 开始拖动擦除状态，记录初始擦除点。
/// - 其他绘图工具类型 (Line, Rectangle, Freehand, NormalEraser, Ellipse, Star):
///   开始一个新的图形绘制过程，创建对应类型的 AbstractShape 对象实例作为预览图形 (currentShapeInProgressPtr)，
///   并记录鼠标按下的点作为绘图的起始点 (tempStartPoint)。
///
/// @param event 指向 QMouseEvent 对象的指针，包含了鼠标事件的详细信息（如按下的按钮、位置等）。
void ArtboardView::mousePressEvent(QMouseEvent *event)
{
    // 仅当按下的是鼠标左键时才处理事件
    if (event->button() == Qt::LeftButton) {

        if (currentShapeType == ShapeType::None) { // 当前处于“选择”工具模式
            AbstractShape* previouslySelectedShape = m_selectedShape; // 记录之前的选中图形，用于判断选中状态是否改变
            m_selectedShape = nullptr; // 每次点击时，默认先清除当前的选中状态（单选模式）
            bool selectionFoundThisClick = false; // 标记本次点击是否选中了任何图形

            // 从图形列表的末尾向前遍历（即从最上层绘制的图形开始检查）
            for (int i = shapesList.size() - 1; i >= 0; --i) {
                AbstractShape* shape = shapesList.at(i);
                // 确保图形指针有效，并调用其 containsPoint 方法判断鼠标点击位置是否在该图形上
                if (shape && shape->containsPoint(event->pos())) {
                    m_selectedShape = shape; // 将该图形设为当前选中图形
                    selectionFoundThisClick = true;
                    qDebug() << "ArtboardView::mousePressEvent (Select): Shape selected. Address:" << m_selectedShape
                             << "Type:" << (m_selectedShape ? m_selectedShape->getType() : ShapeType::None); // 安全获取类型
                    break; // 既然是单选，找到第一个（最顶层的）命中的图形后就停止遍历
                }
            }

            if (!selectionFoundThisClick) {
                qDebug() << "ArtboardView::mousePressEvent (Select): Clicked on empty area, selection (if any) cleared.";
            }

            // 如果选中状态（哪个图形被选中，或者从选中变为未选中，反之亦然）发生了变化，
            // 则请求重绘界面，以便 paintEvent 可以更新选中框的显示。
            if (m_selectedShape != previouslySelectedShape) {
                update();
            }

            // 如果成功选中了一个图形，则设置 isCurrentlyDrawing 为 true，
            // 并记录当前鼠标位置。这为后续可能的拖动操作（在 mouseMoveEvent 中处理）做准备。
            if (m_selectedShape) {
                isCurrentlyDrawing = true;                  // 标记交互操作开始（可能是简单选中，也可能是拖动的开始）
                tempStartPoint = event->pos();              // 用于 mouseMoveEvent 中计算小段位移的参考点
                m_dragStartPoint_forCommand = event->pos(); // 记录本次拖动操作的起始点，用于 MoveCommand 计算总偏移
            } else {
                isCurrentlyDrawing = false; // 没有选中图形，不能进行拖动
            }

            // 在选择模式下，不应该有正在绘制过程中的图形 (currentShapeInProgressPtr)。
            // 作为防御性编程，如果它意外存在，则清理掉。
            if (currentShapeInProgressPtr) {
                delete currentShapeInProgressPtr;
                currentShapeInProgressPtr = nullptr;
            }

        } else if (currentShapeType == ShapeType::StrokeEraser) { // 当前处于“点击笔画橡皮擦”模式
            AbstractShape *shapeHit = nullptr; // 指向被命中的图形
            int hitIndex = -1;                 // 被命中图形在 shapesList 中的索引

            // 从顶层图形开始查找被点击的图形
            for (int i = shapesList.size() - 1; i >= 0; --i) {
                if (shapesList.at(i) && shapesList.at(i)->containsPoint(event->pos())) {
                    shapeHit = shapesList.at(i);
                    hitIndex = i;
                    break;
                }
            }

            if (shapeHit) { // 如果找到了被点击的图形
                // 创建一个 DeleteShapeCommand 对象来封装删除操作
                DeleteShapeCommand *deleteCmd = new DeleteShapeCommand(shapeHit, this, hitIndex);
                // 通过 executeCommand 方法来执行命令、将其压入撤销栈、并清空重做栈
                this->executeCommand(deleteCmd);
                qDebug() << "ArtboardView::mousePressEvent (StrokeEraser): DeleteShapeCommand submitted. Shape Addr:" << shapeHit;
            } else {
                qDebug() << "ArtboardView::mousePressEvent (StrokeEraser): Clicked on empty area or no shape hit.";
            }
            isCurrentlyDrawing = false; // 点击式橡皮擦是一次性操作，不进入拖动状态

        } else if (currentShapeType == ShapeType::DraggingStrokeEraser) { // 当前处于“拖动式笔画橡皮擦”模式
            isCurrentlyDrawing = true;               // 标记开始拖动擦除操作
            shapesToDeleteInCurrentDrag.clear();     // 清空上一次拖动操作可能遗留的待删除图形集合
            performStrokeEraseAtPoint(event->pos()); // 对当前鼠标按下的点，也执行一次擦除检测和标记

        } else { // currentShapeType != ShapeType::None 且不是上面两种笔画橡皮擦，即为“正常的绘图工具”模式
            // (包括 Line, Rectangle, Freehand, NormalEraser, Ellipse, Star)
            isCurrentlyDrawing = true;        // 标记开始绘图操作
            tempStartPoint = event->pos();    // 记录绘图的起始点

            // 清理上一个可能未完成的图形对象（例如，如果上次绘图没有正常释放鼠标）
            // 这是一种防御性措施，确保 currentShapeInProgressPtr 不含奇奇怪怪的内存地址。
            if (currentShapeInProgressPtr) {
                delete currentShapeInProgressPtr;
                currentShapeInProgressPtr = nullptr;
            }

            // 根据当前选择的工具类型 (currentShapeType)，创建对应图形类的实例
            // 并将其指针赋给 currentShapeInProgressPtr，用于后续的实时预览和最终添加。
            switch (currentShapeType) {
            case ShapeType::Line:
                currentShapeInProgressPtr = new LineShape(
                    tempStartPoint, tempStartPoint,      // 初始时起点和终点相同
                    currentDrawingColor, currentPenWidth // 使用当前的边框颜色和线宽
                    // LineShape 构造函数内部会处理填充属性 (设为不填充)
                    );
                break;
            case ShapeType::Rectangle:
                currentShapeInProgressPtr = new RectangleShape(
                    tempStartPoint, tempStartPoint,
                    currentDrawingColor, currentPenWidth,
                    currentIsFilled, currentDrawingFillColor // 传递当前的填充状态和颜色
                    );
                break;
            case ShapeType::Freehand:
                currentShapeInProgressPtr = new FreehandPathShape(
                    QVector<QPoint>() << tempStartPoint, // 初始点集只包含当前按下的点
                    currentDrawingColor, currentPenWidth
                    // FreehandPathShape 构造函数内部会处理填充属性 (设为不填充)
                    );
                break;
            case ShapeType::NormalEraser: // 普通橡皮擦
                currentShapeInProgressPtr = new EraserPathShape(
                    QVector<QPoint>() << tempStartPoint, // 初始点集
                    currentPenWidth,                      // 使用当前线宽作为橡皮擦的粗细
                    palette().window().color()            // 使用画布背景色作为橡皮擦的“绘制”颜色
                    );
                break;
            case ShapeType::Ellipse:
                currentShapeInProgressPtr = new EllipseShape(
                    tempStartPoint, tempStartPoint,
                    currentDrawingColor, currentPenWidth,
                    currentIsFilled, currentDrawingFillColor);
                break;
            case ShapeType::Star:
                currentShapeInProgressPtr = new StarShape(
                    tempStartPoint, tempStartPoint,
                    currentDrawingColor, currentPenWidth,
                    currentIsFilled, currentDrawingFillColor
                    /*, 5 // 默认角点数在 StarShape 构造函数中处理 */
                    );
                break;
            default:
                // 如果 currentShapeType 是一个未知的绘图类型，则不进行操作，并重置状态
                isCurrentlyDrawing = false;
                currentShapeInProgressPtr = nullptr; // 确保指针为空
                qWarning() << "ArtboardView::mousePressEvent - Unexpected drawing shape type for creation:" << currentShapeType;
                break;
            } // switch 结束

            // 如果成功创建了新的正在绘制的图形对象 (currentShapeInProgressPtr 非空)，
            // 则输出相关的调试信息。
            if (currentShapeInProgressPtr) {
                qDebug() << "ArtboardView::mousePressEvent (Drawing Tool): NEW Shape created. Address:" << (void*)currentShapeInProgressPtr
                         << "Type:" << currentShapeType
                         << "Border Color:" << currentShapeInProgressPtr->getBorderColor().name()
                         << "Fill Color:" << currentShapeInProgressPtr->getFillColor().name()
                         << "Is Filled:" << currentShapeInProgressPtr->isFilled()
                         << "Width:" << currentShapeInProgressPtr->getPenWidth();
            } else if (isCurrentlyDrawing) {
                // 如果 isCurrentlyDrawing 被设为 true，但由于某种原因没有创建出图形对象
                // (例如 default case)，则需要重置 isCurrentlyDrawing 状态。
                isCurrentlyDrawing = false;
                qWarning() << "ArtboardView::mousePressEvent - isCurrentlyDrawing was true but no shape instance created for type:" << currentShapeType;
            }
        }
    } else { // 如果按下的不是鼠标左键，则调用基类的默认处理方法
        QWidget::mousePressEvent(event);
    }
} // ArtboardView::mousePressEvent 函数结束


/// @brief 处理鼠标在画布上移动的事件。
///
/// 此函数主要在用户按住鼠标左键并进行拖动操作时被调用 (通过 isCurrentlyDrawing 和 event->buttons() 判断)。
/// 根据当前的 ArtboardView 状态 (currentShapeType)，它会执行不同的实时更新操作：
/// - ShapeType::None (选择工具) 且有图形被选中 (m_selectedShape 非空):
///   实时移动被选中的图形，并更新画布以显示拖动效果。
/// - ShapeType::DraggingStrokeEraser (拖动式笔画橡皮擦):
///   持续调用 performStrokeEraseAtPoint() 来标记鼠标轨迹下需要被删除的图形。
/// - 其他绘图工具类型 (且 currentShapeInProgressPtr 有效):
///   实时更新正在绘制的图形的几何形状 (例如，直线的终点、矩形/椭圆/五角星的对角点、自由曲线/橡皮擦路径的新点)，
///   并请求重绘画布以显示预览效果。
///
/// @param event 指向 QMouseEvent 对象的指针，包含了鼠标事件的详细信息（如当前位置、按下的按钮状态等）。
void ArtboardView::mouseMoveEvent(QMouseEvent *event)
{
    // 首先检查：是否处于一个“正在进行中的操作”状态 (isCurrentlyDrawing is true)，
    // 并且鼠标左键当前是否确实是按下的状态 (event->buttons() & Qt::LeftButton)。
    if (isCurrentlyDrawing && (event->buttons() & Qt::LeftButton)) {
        if (currentShapeType == ShapeType::None && m_selectedShape) {
            // --- 当前是“选择”工具模式，并且有一个图形 (m_selectedShape) 被选中，正在被拖动 ---
            QPoint currentPoint = event->pos(); // 获取当前鼠标位置
            // 计算从上一个鼠标位置 (tempStartPoint) 到当前位置的偏移量 (delta)
            QPoint offset = currentPoint - tempStartPoint;

            if (!offset.isNull()) { // 只有当鼠标确实发生了位移时才进行操作 (避免原地重复计算)
                // 调用选中图形的 moveBy() 方法，让它根据偏移量更新自己的位置。
                // moveBy() 是 AbstractShape 的虚函数，具体实现由派生类 (LineShape, RectangleShape 等) 提供。
                m_selectedShape->moveBy(offset);

                // 更新 tempStartPoint 为当前鼠标位置，
                // 这样下一次 mouseMoveEvent 触发时，就能计算出相对于“这次”位置的新偏移量，
                // 从而实现平滑的、连续的拖动效果。
                tempStartPoint = currentPoint;

                update(); // 请求重绘 ArtboardView，以实时显示被拖动图形的新位置。
                // 通常不需要在这里输出 qDebug()，因为它会非常频繁地触发，影响性能和日志可读性。
                // qDebug() << "ArtboardView: Selected shape moved by" << offset;
            }
        } else if (currentShapeType == ShapeType::DraggingStrokeEraser) {
            // --- 当前是“拖动式笔画橡皮擦”模式 ---
            // 持续调用 performStrokeEraseAtPoint()，传入当前鼠标位置。
            // 这个辅助函数会检查当前点是否命中了任何图形，并将命中的图形标记到
            // shapesToDeleteInCurrentDrag 集合中，以便在 mouseReleaseEvent 中统一删除。
            performStrokeEraseAtPoint(event->pos());
        } else if (currentShapeInProgressPtr) {
            // --- 当前是某个“绘图工具”模式 (Line, Rectangle, Freehand, NormalEraser, Ellipse, Star)，
            //     并且有一个正在绘制过程中的图形对象 (currentShapeInProgressPtr)。---
            // 调用当前正在绘制的图形对象的 updateShape() 方法，传入当前鼠标位置。
            // updateShape() 是 AbstractShape 的虚函数，每个派生类会根据自己的特点来更新其几何形状
            currentShapeInProgressPtr->updateShape(event->pos());

            update(); // 请求重绘 ArtboardView，以实时显示正在绘制的图形的预览效果。
        }
        // 如果 isCurrentlyDrawing 为 true，但以上条件都不满足。例如 currentShapeType 是某个绘图工具
        // 但 currentShapeInProgressPtr 意外为 nullptr，则不执行任何操作，这可以防止潜在的空指针访问。
    } else {
        // 如果鼠标移动时左键没有按下，或者不处于 isCurrentlyDrawing 状态，
        // 则将事件传递给基类 QWidget 进行默认处理。
        // 这对于处理鼠标悬停提示 (tooltips)、鼠标光标形状改变等非拖动交互是重要的。
        // 如果之前为 ArtboardView 设置了 setMouseTracking(true)，那么即使鼠标按钮未按下，
        // mouseMoveEvent 也会被触发，这时这个 else 分支就会执行。
        QWidget::mouseMoveEvent(event);
    }
}

/// @brief 处理鼠标在画布上释放的事件。
///
/// 此函数在用户完成一次鼠标左键拖动操作并释放按钮时被调用。
/// 它的核心职责是根据当前的工具模式 (currentShapeType) 和操作状态 (isCurrentlyDrawing, m_selectedShape, currentShapeInProgressPtr)
/// 来最终确定操作的结果，并创建相应的命令对象，以便支持撤销/重做。
///
/// 主要处理逻辑分支：
/// 1. 选择工具模式 (ShapeType::None) 且有图形被选中并拖动 (m_selectedShape && isCurrentlyDrawing):
///    - 计算总的拖动偏移量。
///    - 如果发生了实际位移，则先将被拖动图形“移回”到拖动开始时的位置，
///      然后创建一个 MoveShapeCommand（存储正向的总偏移量），并通过 executeCommand() 执行。
/// 2. 拖动式笔画橡皮擦模式 (ShapeType::DraggingStrokeEraser) 且正在拖动:
///    - 如果在拖动过程中标记了任何待删除图形 (shapesToDeleteInCurrentDrag 非空)，
///      则为这些图形创建一系列 DeleteShapeCommand，并将它们封装到一个 DeleteMultipleShapesCommand 中。
///    - 通过 executeCommand() 执行这个宏命令。
/// 3. 正常的绘图工具模式 (且 currentShapeInProgressPtr 有效):
///    - 最终确定正在绘制的图形的几何形状。
///    - 进行图形有效性检查（比如说直线长度不能为0，自由曲线至少有2个点）。
///    - 如果图形有效，则创建一个 AddShapeCommand，将正在绘制的图形 (currentShapeInProgressPtr)
///      的所有权转移给该命令，并通过 executeCommand() 执行。
///    - 如果图形无效，则直接 delete currentShapeInProgressPtr。
/// 4. 其他情况 (如非左键释放、操作意外中断等):
///    - 清理可能存在的临时状态 (如 shapesToDeleteInCurrentDrag, currentShapeInProgressPtr)。
///    - 重置 isCurrentlyDrawing 标志。
///    - 更新撤销/重做按钮的可用状态。
///
/// @param event 指向 QMouseEvent 对象的指针，包含了鼠标事件的详细信息。
void ArtboardView::mouseReleaseEvent(QMouseEvent *event)
{
    // 确保是鼠标左键释放，并且之前确实处于一个正在进行中的拖动/绘图操作 (isCurrentlyDrawing 为 true)
    if (event->button() == Qt::LeftButton && isCurrentlyDrawing) {
        if (currentShapeType == ShapeType::None && m_selectedShape) {
            // --- “选择”工具模式下，完成了对选中图形的拖动 ---
            QPoint finalMousePosition = event->pos();
            // 计算从拖动操作开始 (m_dragStartPoint_forCommand) 到当前释放点的总偏移量
            QPoint totalOffsetSincePress = finalMousePosition - m_dragStartPoint_forCommand;

            if (totalOffsetSincePress != QPoint(0, 0)) { // 只有当确实发生了有效的拖动位移时
                qDebug() << "ArtboardView::mouseReleaseEvent (Select): Drag ended. Total offset:" << totalOffsetSincePress;

                // 核心策略：为了让 MoveShapeCommand 的 execute() 和 undo() 逻辑简单对称
                // (即 execute() 执行 moveBy(offset)，undo() 执行 moveBy(-offset))，
                // 我们在这里先将被选中的图形 (m_selectedShape) 通过 moveBy(-totalOffsetSincePress)
                // “恢复”到它在本次拖动操作开始前的原始位置。
                m_selectedShape->moveBy(-totalOffsetSincePress);

                // 然后，创建一个 MoveShapeCommand，它存储的是“正向”的总偏移量 totalOffsetSincePress。
                MoveShapeCommand *moveCmd = new MoveShapeCommand(m_selectedShape, totalOffsetSincePress, this);

                // 通过 executeCommand 来执行这个移动命令。
                // moveCmd->execute() 内部会调用 m_selectedShape->moveBy(totalOffsetSincePress)，
                // 从而将图形从（我们刚刚恢复的）原始位置移动到用户拖动结束的最终位置。
                // executeCommand 还会将 moveCmd 压入撤销栈，并清空重做栈，更新UI按钮状态。
                this->executeCommand(moveCmd);

                qDebug() << "ArtboardView::mouseReleaseEvent (Select): MoveShapeCommand submitted. Shape:" << (void*)m_selectedShape;
            } else {
                // 如果 totalOffsetSincePress 是 (0,0)，意味着用户只是点击选中了图形，并没有显著拖动。
                // 这种情况下，不需要创建 MoveShapeCommand。
                qDebug() << "ArtboardView::mouseReleaseEvent (Select): Selected shape was clicked, no significant drag occurred.";
            }
            isCurrentlyDrawing = false; // 结束本次拖动/选择操作状态
            // m_selectedShape 保持不变 (除非用户在 mousePress 时点击了空白处并清除了它)
            // update(); // executeCommand 内部的命令的 execute 通常会调用 update，这里可能不需要重复。
            // paintEvent 也会因为选中框的显示而需要更新，但 mousePress 中已有 update。
            // 如果只是点击，选中框已经在 mousePress 中通过 update 显示了。
            // 如果拖动了，命令的 execute 会 update。
            // 此处可以考虑是否需要额外的 update 来确保选中框状态的最终一致性。
            // 通常 executeCommand 里的 update 足够了。

        } else if (currentShapeType == ShapeType::DraggingStrokeEraser) {
            // --- “拖动式笔画橡皮擦”模式下，鼠标释放，结束本次拖动擦除 ---
            if (!shapesToDeleteInCurrentDrag.isEmpty()) { // 如果在拖动过程中标记了任何待删除的图形
                qDebug() << "ArtboardView::mouseReleaseEvent (DraggingStrokeEraser): Processing"
                         << shapesToDeleteInCurrentDrag.size() << "shapes for deletion.";

                // 1. 获取所有待删除图形及其在当前 shapesList 中的原始索引，并按索引降序排列。
                //    按索引降序处理是为了在创建单个 DeleteShapeCommand 时，如果命令立即执行（从列表移除）
                //    对于 DeleteMultipleShapesCommand，它内部的子命令执行顺序也很重要。
                QList<QPair<AbstractShape*, int>> sortedShapesAndIndices;
                for (AbstractShape* shapeToMark : shapesToDeleteInCurrentDrag) {
                    int index = shapesList.indexOf(shapeToMark); // 获取图形在当前列表中的索引
                    if (index != -1) { // 确保图形仍然存在于列表中
                        sortedShapesAndIndices.append(qMakePair(shapeToMark, index));
                    }
                }
                // 按索引从大到小排序 (降序)
                std::sort(sortedShapesAndIndices.begin(), sortedShapesAndIndices.end(),
                          [](const QPair<AbstractShape*, int>& a, const QPair<AbstractShape*, int>& b) {
                              return a.second > b.second;
                          });
                //查阅资料，这里使用了lambda表达式

                // 2. 为这些排好序的图形创建一系列的单个 DeleteShapeCommand
                QList<DeleteShapeCommand*> individualDeleteCommands;
                for (const auto &pair : sortedShapesAndIndices) {
                    AbstractShape* shapeToRemove = pair.first;
                    int originalIndex = pair.second; // 这是它在被删除前的索引
                    // 创建单个的 DeleteShapeCommand，这些命令将被下面的宏命令管理其生命周期
                    individualDeleteCommands.append(new DeleteShapeCommand(shapeToRemove, this, originalIndex));
                }

                if (!individualDeleteCommands.isEmpty()) {
                    // 3. 创建一个 DeleteMultipleShapesCommand 来封装这些单个的删除命令
                    DeleteMultipleShapesCommand *macroDeleteCmd = new DeleteMultipleShapesCommand(individualDeleteCommands);

                    // 4. 通过 executeCommand 来执行这个宏命令。
                    //    宏命令的 execute() 会依次执行所有子 DeleteShapeCommand 的 execute()，
                    //    即从 shapesList 中移除相应的图形。
                    //    executeCommand 还会将宏命令压入撤销栈，并清空重做栈。
                    this->executeCommand(macroDeleteCmd);

                    qDebug() << "ArtboardView::mouseReleaseEvent (DraggingStrokeEraser): DeleteMultipleShapesCommand submitted with"
                             << individualDeleteCommands.size() << "sub-commands.";
                }

                shapesToDeleteInCurrentDrag.clear(); // 清空本次拖动操作标记的待删除图形集合
                // update(); // executeCommand 内部的命令的 execute 通常会调用 update，这里可能不需要重复。
                // 如果 DeleteMultipleShapesCommand 的 execute 没有触发 update，则这里需要。
                // 我们假设 executeCommand 会保证最终的 update。
            } else {
                qDebug() << "ArtboardView::mouseReleaseEvent (DraggingStrokeEraser): No shapes were marked for deletion during drag.";
            }
            isCurrentlyDrawing = false; // 结束拖动擦除操作状态

        } else if (currentShapeInProgressPtr) { // --- 正常的“绘图工具”模式下，鼠标释放，完成图形绘制 ---
            currentShapeInProgressPtr->updateShape(event->pos()); // 用鼠标释放的最终位置更新图形的几何形状

            bool shapeIsValid = true; // 标志当前绘制的图形是否有效（例如，直线长度不为0）
            ShapeType currentType = currentShapeInProgressPtr->getType(); // 获取正在绘制图形的类型

            // 对不同类型的图形进行有效性判断
            if (currentType == ShapeType::Line) {
                LineShape* line = static_cast<LineShape*>(currentShapeInProgressPtr);
                if (line && line->getStartPoint() == line->getEndPoint()) { // 如果起点和终点相同
                    shapeIsValid = false;
                }
            } else if (currentType == ShapeType::Freehand) {
                FreehandPathShape* pathShape = dynamic_cast<FreehandPathShape*>(currentShapeInProgressPtr);
                if (pathShape && pathShape->getPoints().size() < 2) { // 自由画笔至少需要2个点
                    shapeIsValid = false;
                }
            } else if (currentType == ShapeType::NormalEraser) { // 普通橡皮擦（用背景色绘制的路径）
                EraserPathShape* eraserPath = dynamic_cast<EraserPathShape*>(currentShapeInProgressPtr);
                if (eraserPath && eraserPath->getPoints().isEmpty()) { // 橡皮擦路径为空则无效
                    shapeIsValid = false;
                }
            } else if (currentType == ShapeType::Ellipse) {
                EllipseShape* ellipse = static_cast<EllipseShape*>(currentShapeInProgressPtr);
                // 椭圆的Surrounding box如果为空或无效（例如宽高为0），则认为图形无效
                if (ellipse && ellipse->getBoundingRect().isEmpty()) {
                    shapeIsValid = false;
                }
            } else if (currentType == ShapeType::Star) {
                StarShape* star = static_cast<StarShape*>(currentShapeInProgressPtr);
                // 五角星的Surrounding box如果为空或无效，则认为图形无效
                if (star && star->getBoundingRect().isEmpty()) {
                    shapeIsValid = false;
                }
            }
            // 对于矩形，即使起点和终点相同（形成一个点），通常也认为是有效的，
            // 或者其 getNormalizedRect() 会处理这种情况，所以一般不需要特判。

            if (shapeIsValid) { // 如果图形被认为是有效的
                qDebug() << "ArtboardView::mouseReleaseEvent (Drawing Tool): Creating AddShapeCommand for shape at address:" << (void*)currentShapeInProgressPtr;
                // 1. 创建一个 AddShapeCommand 对象，将正在绘制的图形 currentShapeInProgressPtr 传递给它。
                //    命令对象将接管这个图形对象的所有权。
                AddShapeCommand *addCmd = new AddShapeCommand(currentShapeInProgressPtr, this);

                // 2. 将 ArtboardView 的 currentShapeInProgressPtr 置为空指针。
                //    这是非常重要的一步，因为它表明图形对象的所有权已经转移给了 addCmd 命令，
                //    ArtboardView 不再直接持有或管理这个“正在进行中”的图形了。
                //    这样可以防止在下一次 mousePressEvent 中意外地 delete 掉这个已经被命令拥有的对象。
                AbstractShape* shapeJustAdded = currentShapeInProgressPtr; // 临时保存一下指针用于调试输出
                currentShapeInProgressPtr = nullptr;

                // 3. 通过 executeCommand 方法来执行这个添加命令。
                //    executeCommand 会调用 addCmd->execute()（将图形添加到 shapesList），
                //    然后将 addCmd 压入 undoStack，并清空 redoStack，同时更新UI按钮状态。
                this->executeCommand(addCmd);

                qDebug() << "ArtboardView: AddShapeCommand submitted via executeCommand for shape type"
                         << (shapeJustAdded ? shapeJustAdded->getType() : ShapeType::None);
            } else { // 如果图形无效（例如太小或不符合规则）
                delete currentShapeInProgressPtr; // 直接删除这个未完成的、无效的图形对象
                currentShapeInProgressPtr = nullptr; // 将指针置空
                qDebug() << "ArtboardView::mouseReleaseEvent (Drawing Tool): Shape was invalid or too small, discarded.";
                updateUndoRedoStatus(); // 由于没有产生新的有效命令，也需要更新一下撤销/重做按钮的状态
            }
            isCurrentlyDrawing = false; // 结束本次绘图操作状态
            // update(); // 通常 addCmd->execute() 内部已经调用了 update()，这里可能不需要重复。
            // 如果图形无效被丢弃，可能需要一次 update() 来清除可能残留的预览痕迹。
        } else {
            // 如果 isCurrentlyDrawing 为 true，但 currentShapeInProgressPtr 为空
            // (例如，从选择模式或橡皮擦模式意外进入此分支，或者 mousePress 未成功创建对象)
            // 这算是一种异常情况，重置 isCurrentlyDrawing 状态。
            isCurrentlyDrawing = false;
            qDebug() << "ArtboardView::mouseReleaseEvent - isCurrentlyDrawing was true, but no currentShapeInProgressPtr. Resetting state.";
            updateUndoRedoStatus(); // 确保按钮状态与实际栈状态一致
        }
    } else { // 如果不是鼠标左键释放，或者 isCurrentlyDrawing 本来就是 false (例如，只是单击而没有拖动，且非绘图/选择操作)
        // 处理操作被意外中断的情况 (例如，用户在拖动时按下了鼠标右键，或者窗口失去焦点等)
        if (currentShapeType == ShapeType::DraggingStrokeEraser && isCurrentlyDrawing) {
            // 如果是拖动擦除操作被中断，清空已标记的待删除图形列表，避免下次误删
            if (!shapesToDeleteInCurrentDrag.isEmpty()) {
                qDebug() << "ArtboardView::mouseReleaseEvent - DraggingStrokeEraser operation cancelled mid-drag, clearing marked shapes.";
                shapesToDeleteInCurrentDrag.clear();
            }
        }
        if (currentShapeInProgressPtr && isCurrentlyDrawing) { // 如果有正在绘制的图形但操作被中断
            qDebug() << "ArtboardView::mouseReleaseEvent - Drawing operation cancelled mid-drag, discarding shape in progress.";
            delete currentShapeInProgressPtr; // 删除这个未完成的图形
            currentShapeInProgressPtr = nullptr;
        }
        isCurrentlyDrawing = false; // 无论如何，确保 isCurrentlyDrawing 状态被重置
        updateUndoRedoStatus();     // 更新撤销/重做按钮的状态
        QWidget::mouseReleaseEvent(event); // 调用基类方法处理其他可能的事件（例如上下文菜单）
    }
}

/// @brief 设置当前绘图操作使用的边框颜色。
/// @param color 要设置的 QColor 对象，代表新的边框颜色。
void ArtboardView::setCurrentDrawingColor(const QColor &color)
{
    qDebug() << "ArtboardView::setCurrentDrawingColor - Attempting to set border color to:" << color.name();
    // 仅当新颜色与当前边框颜色不同时才进行更新，以避免不必要的操作。
    if (this->currentDrawingColor != color) {
        this->currentDrawingColor = color; // 更新存储当前边框颜色的成员变量
        qDebug() << "ArtboardView: Actual current border color set to" << this->currentDrawingColor.name();
    }
}

/// @brief 设置当前绘图操作使用的线宽（画笔粗细）。
/// @param width 要设置的线宽值（像素）。为了保证绘制有效，线宽必须大于0。
void ArtboardView::setCurrentPenWidth(int width)
{
    qDebug() << "ArtboardView::setCurrentPenWidth - Attempting to set pen width to:" << width;
    // 仅当新线宽与当前线宽不同，并且新线宽是有效值（大于0）时才更新。
    if (this->currentPenWidth != width && width > 0) {
        this->currentPenWidth = width; // 更新存储当前线宽的成员变量
        qDebug() << "ArtboardView: Actual current pen width set to" << this->currentPenWidth;
    }
}

/// @brief 设置当前绘图操作使用的填充颜色。
/// 此方法同时会根据传入颜色是否具有透明度来智能更新 `currentIsFilled` 标志：
/// - 如果选择的填充颜色是非完全透明的 (alpha > 0)，则自动启用填充模式 (`currentIsFilled = true`)。
/// - 如果选择的填充颜色是完全透明的 (alpha == 0)，则自动禁用填充模式 (`currentIsFilled = false`)。
/// @param color 要设置的填充 QColor 对象。
void ArtboardView::setCurrentDrawingFillColor(const QColor &color)
{
    // 仅当新填充颜色与当前填充颜色不同时才进行更新。
    if (currentDrawingFillColor != color) {
        currentDrawingFillColor = color; // 更新存储当前填充颜色的成员变量

        // 根据新填充颜色的透明度自动更新是否启用了填充的标志。
        if (color.alpha() != 0) { // color.alpha() 返回颜色的alpha分量 (0 表示完全透明, 255 表示完全不透明)
            currentIsFilled = true; // 如果颜色不透明，则认为用户想要填充
        } else {
            currentIsFilled = false; // 如果颜色完全透明，则认为用户不想要填充
        }
        qDebug() << "ArtboardView: Current fill color set to" << currentDrawingFillColor.name()
                 << "Fill enabled (auto-set based on color alpha):" << currentIsFilled;
    }
}

/// @brief 显式设置当前是否启用图形填充模式。
/// 此方法通常由UI控件（如复选框）调用，让用户直接控制是否填充。
/// 它会覆盖 `setCurrentDrawingFillColor` 中基于颜色透明度的自动设置。
/// @param enable true 表示启用填充，false 表示禁用填充。
void ArtboardView::enableFill(bool enable)
{
    // 仅当新的启用状态与当前状态不同时才更新。
    if (currentIsFilled != enable) {
        currentIsFilled = enable; // 更新存储是否填充的成员变量
        qDebug() << "ArtboardView: Fill enabled explicitly set to" << currentIsFilled;
    }
}

/// @brief 执行撤销操作。
/// 如果撤销栈 (undoStack) 非空，则：
/// 1. 从 undoStack 弹出一个命令对象。
/// 2. 调用该命令对象的 undo() 方法，以恢复到该命令执行前的状态。
/// 3. 将该命令对象压入重做栈 (redoStack)，以便后续可以重做。
/// 4. 更新撤销/重做按钮的可用状态 (通过 updateUndoRedoStatus())。
/// @note 假设命令的 undo() 方法内部会负责调用 ArtboardView::update() 来刷新视图。
void ArtboardView::undo()
{
    qDebug() << "ArtboardView::undo() - ENTRY: undoStack size =" << undoStack.size() << ", redoStack size =" << redoStack.size();
    if (!undoStack.isEmpty()) { // 确保撤销栈中有命令可供撤销
        AbstractCommand *commandToUndo = undoStack.pop(); // 从撤销栈顶部弹出一个命令
        qDebug() << "ArtboardView::undo() - Popped command:" << (void*)commandToUndo << "from undoStack.";

        commandToUndo->undo(); // 执行命令的撤销逻辑

        redoStack.push(commandToUndo); // 将此命令移至重做栈
        qDebug() << "ArtboardView::undo() - Pushed command:" << (void*)commandToUndo << "to redoStack.";

        updateUndoRedoStatus(); // 更新UI上撤销/重做按钮的启用/禁用状态

        qDebug() << "ArtboardView::undo() - EXIT: undoStack size =" << undoStack.size() << ", redoStack size =" << redoStack.size();
    } else {
        qDebug() << "ArtboardView: Undo stack is empty. Nothing to undo.";
    }
}

/// @brief 执行重做操作。
/// 如果重做栈 (redoStack) 非空，则：
/// 1. 从 redoStack 弹出一个命令对象。
/// 2. 调用该命令对象的 execute() 方法，以重新执行该命令。
/// 3. 将该命令对象压回撤销栈 (undoStack)。
/// 4. 更新撤销/重做按钮的可用状态 (通过 updateUndoRedoStatus())。
/// @note 假设命令的 execute() 方法内部会负责调用 ArtboardView::update() 来刷新视图。
void ArtboardView::redo()
{
    qDebug() << "ArtboardView::redo() - ENTRY: undoStack size =" << undoStack.size() << ", redoStack size =" << redoStack.size();
    if (!redoStack.isEmpty()) { // 确保重做栈中有命令可供重做
        AbstractCommand *commandToRedo = redoStack.pop(); // 从重做栈顶部弹出一个命令
        qDebug() << "ArtboardView::redo() - Popped command:" << (void*)commandToRedo << "from redoStack.";

        commandToRedo->execute(); // 执行命令的 execute 逻辑 (即重做)

        undoStack.push(commandToRedo); // 将此命令移回撤销栈
        qDebug() << "ArtboardView::redo() - Pushed command:" << (void*)commandToRedo << "to undoStack.";

        updateUndoRedoStatus(); // 更新UI上撤销/重做按钮的启用/禁用状态

        qDebug() << "ArtboardView::redo() - EXIT: undoStack size =" << undoStack.size() << ", redoStack size =" << redoStack.size();
    } else {
        qDebug() << "ArtboardView: Redo stack is empty. Nothing to redo.";
    }
}

/// @brief 清空并释放撤销栈 (undoStack) 和重做栈 (redoStack) 中的所有命令对象。
/// 此方法会 `delete` 栈中存储的 `AbstractCommand` 指针所指向的对象，以防止内存泄漏。
/// 操作完成后会调用 `updateUndoRedoStatus()` 来更新UI中撤销/重做按钮的状态。
void ArtboardView::clearCommandStacks()
{
    qDebug() << "ArtboardView::clearCommandStacks() - Clearing both undo and redo stacks.";
    // 清理撤销栈：依次弹出并删除每个命令对象
    while (!undoStack.isEmpty()) {
        delete undoStack.pop(); // pop() 返回栈顶元素并将其从栈中移除，然后 delete 该命令对象
    }
    // 清理重做栈：依次弹出并删除每个命令对象
    while (!redoStack.isEmpty()) {
        delete redoStack.pop();
    }
    // 两种栈都已清空，更新UI上撤销/重做按钮的可用状态
    updateUndoRedoStatus();
}

/// @brief 根据当前撤销栈和重做栈是否为空，发射相应的信号来更新UI。
/// `undoAvailabilityChanged(bool)`: 当撤销栈非空时发射 true，表示可以撤销。
/// `redoAvailabilityChanged(bool)`: 当重做栈非空时发射 true，表示可以重做。
/// 此方法在命令栈状态可能发生改变的各个地方被调用。
void ArtboardView::updateUndoRedoStatus()
{
    bool undoAvailable = !undoStack.isEmpty();
    bool redoAvailable = !redoStack.isEmpty();

    emit undoAvailabilityChanged(undoAvailable); // 发射撤销可用性改变信号
    emit redoAvailabilityChanged(redoAvailable); // 发射重做可用性改变信号

    qDebug() << "ArtboardView::updateUndoRedoStatus() - Undo available:" << undoAvailable
             << "| Redo available:" << redoAvailable;
}


/// @brief 清空并释放重做栈 (redoStack) 中的所有命令对象。
/// 通常在执行一个新的可撤销操作 (`executeCommand`) 后调用，
/// 因为新的用户操作会使之前所有可“重做”的历史失效。
/// 操作完成后会调用 `updateUndoRedoStatus()` 来更新UI状态。
void ArtboardView::clearRedoStack()
{
    bool wasNotEmpty = !redoStack.isEmpty(); // 记录在清空前 redoStack 是否为空
    if (wasNotEmpty) { // 仅当栈中确实有内容时才执行清空和输出日志
        qDebug() << "ArtboardView::clearRedoStack() - Clearing redo stack (it was not empty). Size:" << redoStack.size();
        while (!redoStack.isEmpty()) {
            delete redoStack.pop(); // 弹出并删除每个命令对象
        }
    }
    // 无论 redoStack 之前是否为空，都调用 updateUndoRedoStatus()
    // 来确保UI状态（特别是重做按钮的可用性）与当前栈状态同步。
    updateUndoRedoStatus();
}


/// @brief 将当前画布的完整内容（包括默认背景色、加载的背景图片和所有矢量图形）
/// 渲染到一个新的 QImage 对象中。主要用于“导出图片”功能。
/// @return 包含当前画布视觉内容的 QImage 对象。如果视图大小无效，则返回空 QImage。
QImage ArtboardView::renderToImage()
{
    // 获取当前 ArtboardView 的尺寸
    QSize viewSize = this->size();
    if (!viewSize.isValid() || viewSize.isEmpty()) {
        qWarning("ArtboardView::renderToImage() - View size is invalid or empty, returning null image.");
        return QImage(); // 尺寸无效，返回空图像
    }

    // 1. 创建一个与 ArtboardView 当前大小和推荐的绘图格式相同的 QImage 对象
    QImage imageToRender(viewSize, QImage::Format_ARGB32_Premultiplied);

    // 2. 创建一个 QPainter，使其绘图目标设备为我们刚刚创建的 imageToRender
    QPainter painter(&imageToRender);
    painter.setRenderHint(QPainter::Antialiasing, true); // 设置抗锯齿以获得更好的图像质量

    // 3. 绘制最底层的默认画布背景色 (例如白色)
    //    这确保了即使背景图片有透明区域或者根本没有背景图片时，导出的图像也有一个确定的底色。
    painter.fillRect(imageToRender.rect(), palette().window().color());

    // 4. 绘制已加载的背景图片 (m_backgroundImage)，如果存在的话
    //    它会覆盖在上面绘制的默认背景色之上。
    if (m_hasBackgroundImage && !m_backgroundImage.isNull()) {
        // 将背景图片绘制到目标图像的整个区域，保持宽高比并居中
        QRectF targetRect = imageToRender.rect();
        QImage scaledImage = m_backgroundImage.scaled(targetRect.size().toSize(),
                                                      Qt::KeepAspectRatio,
                                                      Qt::SmoothTransformation);
        qreal x = targetRect.left() + (targetRect.width() - scaledImage.width()) / 2.0;
        qreal y = targetRect.top() + (targetRect.height() - scaledImage.height()) / 2.0;
        painter.drawImage(QPointF(x, y), scaledImage);
    }

    // 5. 绘制所有已存储在 shapesList 中的矢量图形对象
    for (AbstractShape *shape : shapesList) {
        if (shape) { // 确保指针有效
            shape->draw(&painter); // 调用每个图形的 draw 方法，将图形绘制到 imageToRender 上
        }
    }

    qDebug() << "ArtboardView: Canvas content rendered to QImage of size" << imageToRender.size();
    return imageToRender;
}

/// @brief 执行一个给定的命令，并将其集成到撤销/重做系统中。
///
/// 这是所有可撤销操作（如添加图形、删除图形、移动图形、清空画布等）
/// 被实际执行并纳入历史记录的核心入口。
/// 步骤如下：
/// 1. 执行传入命令的 `execute()` 方法，使操作生效。
/// 2. 将该命令对象压入撤销栈 (`undoStack`)，以便后续可以撤销。
/// 3. 清空重做栈 (`redoStack`)，因为任何新的可撤销操作都会使之前的重做历史失效。
///    (clearRedoStack 内部会调用 updateUndoRedoStatus 来更新UI按钮状态)。
///
/// @param command 指向要执行的 AbstractCommand 对象的指针。
///                ArtboardView 将通过 undoStack 对其进行后续的生命周期管理（当栈被清理时）。
void ArtboardView::executeCommand(AbstractCommand *command)
{
    if (!command) { // 安全检查，防止传入空指针命令
        qWarning("ArtboardView::executeCommand() - Attempted to execute a null command.");
        return;
    }

    qDebug() << "ArtboardView::executeCommand - BEFORE: undoStack size =" << undoStack.size()
             << ", redoStack size =" << redoStack.size() << "| Executing command:" << (void*)command;

    command->execute();      // 1. 执行命令的具体操作 (这内部通常会调用 ArtboardView::update())

    undoStack.push(command); // 2. 将命令压入撤销栈

    // 3. 清空重做栈。clearRedoStack() 方法内部会负责删除旧的重做命令对象，
    //    并调用 updateUndoRedoStatus() 来更新撤销/重做按钮的启用状态。
    clearRedoStack();

    qDebug() << "ArtboardView::executeCommand - AFTER: undoStack size =" << undoStack.size()
             << ", redoStack size =" << redoStack.size(); // 此时 redoStack 应该为空
}

/// @brief 设置 ArtboardView 的背景图片。
///
/// 此函数接收一个 QImage 对象作为新的背景。
/// - 如果传入的 `image` 有效 (非空)，它将被设置为当前背景图片，
///   并转换为推荐的绘图格式 (ARGB32_Premultiplied) 以优化性能和兼容性。
///   `m_hasBackgroundImage` 标志将被设为 true。
/// - 如果传入的 `image` 为空 (`isNull()`)，并且当前存在背景图片，
///   则会清除当前的背景图片，`m_hasBackgroundImage` 标志将被设为 false。
/// - 无论是设置新背景还是清除背景，操作完成后都会调用 `update()` 来请求重绘视图。
///
/// @param image 要设置为背景的 QImage 对象。
void ArtboardView::setBackgroundImage(const QImage &image)
{
    if (image.isNull()) { // 检查传入的图像是否为空或无效
        // 如果传入空图像，且当前确实有背景图，则执行清除操作
        if (m_hasBackgroundImage) {
            m_backgroundImage = QImage(); // 将内部存储的背景图片对象重置为空 QImage
            m_hasBackgroundImage = false;   // 更新标志位，表示现在没有背景图片
            qDebug() << "ArtboardView::setBackgroundImage - Received null image, background cleared.";
            update(); // 请求重绘界面，以移除旧的背景图片并显示默认背景色
        } else {
            // 如果之前就没有背景图片，并且传入的也是空图像，则无需执行清除操作，但可以记录日志
            qDebug() << "ArtboardView::setBackgroundImage - Attempted to set a null image, but no background was present anyway.";
        }
    } else { // 如果传入的图像是有效的
        // 将传入的图像转换为 Format_ARGB32_Premultiplied 格式。
        // 这是一种推荐的、带有alpha通道且预乘alpha的图像格式，
        // 有助于提高后续绘制性能和避免在半透明合成时出现颜色混合问题。
        m_backgroundImage = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        m_hasBackgroundImage = true; // 更新标志位，表示现在有背景图片了
        qDebug() << "ArtboardView::setBackgroundImage - Background image set. Original size:" << image.size()
                 << "Converted format:" << m_backgroundImage.format();
        update(); // 请求重绘界面，以显示新设置的背景图片
    }
}

/// @brief 清除当前 ArtboardView 画布上已加载的背景图片。
///
/// 如果当前存在背景图片 (`m_hasBackgroundImage` 为 true)，此方法会：
/// 1. 将内部存储的 `m_backgroundImage` 对象重置为空 QImage。
/// 2. 将 `m_hasBackgroundImage` 标志位设为 false。
/// 3. 调用 `update()` 请求画布重绘，此时将不再绘制背景图片，而是显示默认的画布背景色。
/// 如果当前没有背景图片，则此方法不执行任何操作。
void ArtboardView::clearBackgroundImage()
{
    if (m_hasBackgroundImage) { // 仅当确实存在背景图片时才执行清除操作
        m_backgroundImage = QImage(); // 将背景图片成员变量重置为空 QImage
        m_hasBackgroundImage = false;   // 更新标志位，表示现在没有背景图片
        update();                     // 请求重绘界面，以移除背景图片并显示默认背景色
        qDebug() << "ArtboardView: Background image cleared successfully.";
    } else {
        qDebug() << "ArtboardView: No background image was present to clear.";
    }
}



