// ---------------------------------------------------------------------------
// 描述: MoveShapeCommand 类的实现文件。
//       包含了移动图形命令的构造函数、execute() 和 undo() 方法的具体实现。
// ---------------------------------------------------------------------------

#include "moveshapecommand.h"
#include "artboardview.h" // 如果需要调用 view->update()
#include <QDebug>         // 用于调试输出

/// @brief MoveShapeCommand 构造函数的实现。
/// @param shapeToMove 指向要移动的 AbstractShape 对象的指针。
/// @param moveOffset 本次移动的净偏移量。
/// @param view 指向 ArtboardView 实例的指针。
MoveShapeCommand::MoveShapeCommand(AbstractShape *shapeToMove, const QPoint &moveOffset, ArtboardView *view)
    : m_shapeMoved(shapeToMove), // 初始化，存储被移动的图形对象
    m_offset(moveOffset),      // 初始化，存储移动的偏移量
    m_artboardView(view)       // 初始化，存储 ArtboardView 的指针
{
    // qDebug() << "MoveShapeCommand CONSTRUCTOR: For shape" << (void*)m_shapeMoved << "with offset" << m_offset;
}

// 析构函数在头文件中已内联定义为 {}，因为命令不拥有 m_shapeMoved。

/// @brief 执行“移动图形”命令。
/// 调用被移动图形 (m_shapeMoved) 的 moveBy() 方法，将其按 m_offset 指定的偏移量移动。
/// 此方法主要用于“重做”操作，或者在标准的“执行命令”流程中（如果图形尚未被移动到最终位置）。
/// 在我们的 ArtboardView::mouseReleaseEvent 实现中，图形在创建此命令前已被“预先移回”其原始位置，
/// 因此这里的 execute() 会将其“正确地”移动到用户拖动结束的目标位置。
void MoveShapeCommand::execute()
{
    if (!m_shapeMoved) {
        qWarning("MoveShapeCommand::execute() - Shape to move is null.");
        return;
    }

    m_shapeMoved->moveBy(m_offset); // 调用图形自身的 moveBy 方法执行移动

    if (m_artboardView) { // 如果 ArtboardView 指针有效
        m_artboardView->update(); // 请求视图重绘以显示移动后的图形
    }
    qDebug() << "MoveShapeCommand: Executed - Shape" << (void*)m_shapeMoved << "moved by" << m_offset;
}

/// @brief 撤销“移动图形”命令。
/// 调用被移动图形 (m_shapeMoved) 的 moveBy() 方法，但使用 m_offset 的相反向量，
/// 从而将图形恢复到其执行此移动命令之前的位置。
void MoveShapeCommand::undo()
{
    if (!m_shapeMoved) {
        qWarning("MoveShapeCommand::undo() - Shape to move back is null.");
        return;
    }

    // QPoint 的一元负号操作符会返回一个新的 QPoint，其 x 和 y 坐标都取反。
    m_shapeMoved->moveBy(-m_offset); // 应用相反的偏移量以移回原位

    if (m_artboardView) { // 如果 ArtboardView 指针有效
        m_artboardView->update(); // 请求视图重绘以显示恢复位置后的图形
    }
    qDebug() << "MoveShapeCommand: Undone - Shape" << (void*)m_shapeMoved << "moved back by" << -m_offset;
}
