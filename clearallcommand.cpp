// ---------------------------------------------------------------------------
// 描述: ClearAllCommand 类的实现文件。
//       包含了清空画布命令的构造函数、析构函数、execute() 和 undo() 方法的具体实现。
// ---------------------------------------------------------------------------

#include "clearallcommand.h"
#include "artboardview.h" // 需要 ArtboardView 的完整定义，以便访问其 shapesList (通过友元) 或调用其方法
#include "abstractshape.h" // m_clearedShapes 中存储的是 AbstractShape*
#include <QDebug>         // 用于调试输出

/// @brief ClearAllCommand 构造函数的实现。
/// @param view 指向 ArtboardView 实例的指针。
ClearAllCommand::ClearAllCommand(ArtboardView *view)
    : m_artboardView(view)
// m_clearedShapes (QVector) 会被默认构造为空列表
{
    // qDebug() << "ClearAllCommand CONSTRUCTOR: Created for ArtboardView:" << (void*)m_artboardView;
}

/// @brief ClearAllCommand 析构函数的实现。
/// 负责在命令对象本身被销毁时，清理其内部可能还持有的图形对象。
/// 如果 m_clearedShapes 列表不为空（通常意味着命令被执行了清空操作，
/// 但之后没有被成功撤销，而命令栈被清理导致此命令对象被删除），
/// 则需要 delete 该列表中的所有 AbstractShape 对象以防止内存泄漏。
ClearAllCommand::~ClearAllCommand()
{
    if (!m_clearedShapes.isEmpty()) { // 检查备份列表是否还有图形
        qDebug() << "ClearAllCommand Destructor: Deleting" << m_clearedShapes.size()
                 << "cleared shapes that were not restored.";
        // qDeleteAll 是 Qt 提供的便捷函数，会遍历容器中的每个指针并对其调用 delete。
        qDeleteAll(m_clearedShapes);
        m_clearedShapes.clear(); // 清空列表本身（移除所有现在是空悬的指针）
    }
}

/// @brief 执行“清空所有图形”命令。
/// 将 ArtboardView 当前 `shapesList` 中的所有图形对象的指针转移到本命令的
/// `m_clearedShapes` 列表中进行备份，然后清空 ArtboardView 的 `shapesList`，
/// 最后请求 ArtboardView 更新其显示。
void ClearAllCommand::execute()
{
    if (!m_artboardView) { // 安全检查
        qWarning("ClearAllCommand::execute() - ArtboardView is null.");
        return;
    }

    qDebug() << "ClearAllCommand: Executing - Clearing all shapes from view. Backing up"
             << m_artboardView->shapesList.size() << "shapes.";

    // 1. 将 ArtboardView 当前的 shapesList 中的内容“转移”到 m_clearedShapes。
    //    这里是直接用 QVector 的赋值操作符，它会进行深拷贝（对于指针是拷贝指针值）。
    //    重要的是，ClearAllCommand 现在逻辑上“拥有”了这些被清空的图形对象。
    //    ArtboardView 的 shapesList 接下来会被清空。
    m_clearedShapes = m_artboardView->shapesList; // 备份当前的所有图形指针

    // 2. 清空 ArtboardView 的实际图形列表。
    //    注意：这里只清空了列表中的指针，并没有 delete 图形对象，因为它们已被 m_clearedShapes“接管”。
    m_artboardView->shapesList.clear();           // 通过友元直接访问并清空

    // 3. 请求 ArtboardView 重绘，此时画布上将不再显示任何图形（背景图除外）。
    m_artboardView->update();
}

/// @brief 撤销“清空所有图形”命令（即恢复所有之前被清空的图形）。
/// 将本命令在 `execute()` 时备份在 `m_clearedShapes` 列表中的所有图形对象的指针
/// 移回到 ArtboardView 的 `shapesList` 中，然后清空本命令的 `m_clearedShapes` 列表
/// （因为图形的所有权已交还给 ArtboardView），最后请求 ArtboardView 更新其显示。
void ClearAllCommand::undo()
{
    if (!m_artboardView) { // 安全检查
        qWarning("ClearAllCommand::undo() - ArtboardView is null.");
        return;
    }

    // 检查 m_clearedShapes 是否真的有内容可以恢复。
    // 如果 execute() 时 shapesList 本来就是空的，那么 m_clearedShapes 也会是空的。
    if (m_clearedShapes.isEmpty() && !m_artboardView->shapesList.isEmpty()) {
        // 如果备份列表是空的，但视图列表不是空的，这可能表示一个不一致的状态
        // （例如，在清空后又执行了其他添加操作，然后才撤销清空）。
        // 这种情况下，直接用空列表覆盖视图列表可能不是期望的。
        // 但标准的“撤销清空”应该是恢复到“清空前”的状态。
        // 我们假设 `execute()` 必定发生在 `undo()` 之前，且 `m_clearedShapes` 保存了正确的状态。
        qDebug() << "ClearAllCommand: Undoing - Restoring 0 shapes (backup was empty) to a non-empty view. View will be cleared first.";
        // 为了确保恢复到“清空前”的状态，如果当前 shapesList 非空，应该先清空它。
        // 但这可能与更复杂的命令序列交互产生问题。
        // 一个更简单的假设是：undo() 总是将 m_clearedShapes 的内容赋给 shapesList。
        // 如果 m_clearedShapes 为空，shapesList 也会变空。
        // qDeleteAll(m_artboardView->shapesList); // 如果需要先删除当前内容
        // m_artboardView->shapesList.clear();
    } else if (m_clearedShapes.isEmpty()) {
        qDebug() << "ClearAllCommand: Undoing - No shapes in backup to restore.";
        // ArtboardView的shapesList可能已经是空的，或者我们不改变它。
        // 无论如何，m_clearedShapes 已空，所有权已转移。
        m_artboardView->update(); // 确保视图刷新（即使没有内容变化）
        return;
    }


    qDebug() << "ClearAllCommand: Undoing - Restoring" << m_clearedShapes.size() << "shapes to view.";

    // 1. 将备份在 m_clearedShapes 中的图形列表内容恢复到 ArtboardView 的 shapesList。
    //    我们假设在调用 undo() 之前，ArtboardView 的 shapesList 应该是空的（因为 execute() 刚清空了它）。
    //    如果不是，直接赋值会覆盖掉 shapesList 中现有的内容。对于 ClearAllCommand 的 undo 来说，
    //    这通常是期望的行为——恢复到“清空”操作之前的状态。
    if (!m_artboardView->shapesList.isEmpty()) {
        // 这是一个警告，表明在撤销“清空”之前，画布上又有了其他图形。
        // 这通常发生在“清空”之后，用户又画了新图形，然后再撤销“清空”。
        // 此时，标准行为应该是“清空”操作之前的图形被恢复，而之后画的新图形应该不受影响（即它们还在）。
        // 所以，我们不能简单地用 m_clearedShapes 覆盖 shapesList。
        // 我们应该将 m_clearedShapes 的内容追加到 shapesList，但这会改变恢复的顺序。
        // 正确的“撤销清空”应该是：将 m_clearedShapes 的内容设为 shapesList 的内容，
        // 并且之前在 shapesList 中的内容（即“清空”之后画的新图形）应该被保留。
        // 这使得 ClearAllCommand 的 undo 变得复杂。

        // **简化的标准行为：** 撤销“清空”就是恢复“清空”那一刻的状态。
        // 如果“清空”后又画了图，这些新图在“撤销清空”时会丢失，除非我们有更复杂的命令管理。
        // 我们目前的模型是：ClearAllCommand::execute() -> shapesList变空。
        // ClearAllCommand::undo() -> shapesList 恢复为 m_clearedShapes 的内容。
        // 如果在 ClearAllCommand::execute() 之后、其 undo() 之前，shapesList 又被 AddShapeCommand 修改了，
        // 那么 ClearAllCommand::undo() 时，shapesList = m_clearedShapes 会覆盖掉那些新加的图形。
        // 这是命令模式中需要仔细考虑的交互问题。

        // **当前实现：简单覆盖。**
        qWarning() << "ClearAllCommand::undo() - ArtboardView's shapesList was not empty before restoring."
                   << "It contained" << m_artboardView->shapesList.size() << "shapes which will be overwritten by the"
                   << m_clearedShapes.size() << "restored shapes.";
        qDeleteAll(m_artboardView->shapesList); // 先删除当前列表中的内容，以避免内存泄漏
        m_artboardView->shapesList.clear();
    }

    m_artboardView->shapesList = m_clearedShapes; // 将备份的图形列表指针复制回 ArtboardView 的主列表

    // 2. 清空本命令内部的 m_clearedShapes 列表，因为这些图形对象的所有权已经交还给了 ArtboardView。
    //    注意：这里只清空指针列表，而不 delete 对象，因为对象已经“还给”了 shapesList。
    m_clearedShapes.clear();

    // 3. 请求 ArtboardView 重绘以显示恢复的图形。
    m_artboardView->update();
}
