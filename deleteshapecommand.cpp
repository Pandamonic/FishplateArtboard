// ---------------------------------------------------------------------------
// 描述: DeleteShapeCommand 类的实现文件。
//       包含了删除单个图形命令的构造函数、析构函数、execute() 和 undo() 方法的具体实现。
// ---------------------------------------------------------------------------

#include "deleteshapecommand.h"
#include "artboardview.h" // 需要 ArtboardView 的完整定义，以便访问其 shapesList (通过友元) 或调用其方法
#include <QDebug>         // 用于调试输出

/// @brief DeleteShapeCommand 构造函数的实现。
/// @param shapeToDelete 指向要删除的 AbstractShape 对象的指针。
/// @param view 指向 ArtboardView 实例的指针。
/// @param index 图形在 ArtboardView 的 shapesList 中的原始索引。
DeleteShapeCommand::DeleteShapeCommand(AbstractShape *shapeToDelete, ArtboardView *view, int index)
    : m_shapeToDelete(shapeToDelete),       // 初始化，存储要删除的图形对象
    m_artboardView(view),               // 初始化，存储 ArtboardView 的指针
    m_originalIndex(index),             // 初始化，存储图形的原始索引
    m_isShapeOwnedByList(true)          // 关键初始化：命令刚创建时，图形还在视图的列表中，
    // 因此视图（或其列表）“拥有”或管理着它，此标志设为 true。
    // 当 execute() 执行后，图形会从列表移除，此标志将变为 false。
{
    // qDebug() << "DeleteShapeCommand CONSTRUCTOR: For shape at" << (void*)m_shapeToDelete
    //          << "Original index:" << m_originalIndex;
}

/// @brief DeleteShapeCommand 析构函数的实现。
/// 负责在命令对象本身被销毁时，正确处理其持有的图形对象 m_shapeToDelete 的内存。
DeleteShapeCommand::~DeleteShapeCommand()
{
    // 核心内存管理逻辑：
    // 如果 m_shapeToDelete 指针有效（非空），并且 m_isShapeOwnedByList 标志为 false
    // (意味着图形当前不在 ArtboardView 的 shapesList 中，通常发生在命令被 execute() 执行后，
    //  然后这个命令对象从 undoStack 中被永久移除并销毁，例如因为新的操作导致撤销历史被截断)，
    // 那么此 DeleteShapeCommand 对象在被销毁时，就有责任 delete 它所持有的 m_shapeToDelete，
    // 因为这个图形已经被从视图中“永久”删除了。
    if (m_shapeToDelete && !m_isShapeOwnedByList) {
        delete m_shapeToDelete;
        m_shapeToDelete = nullptr; // 将指针置空，好习惯
        qDebug() << "DeleteShapeCommand Destructor: Shape at" << (void*)m_shapeToDelete << "deleted (was not in list, assumed deleted by command).";
    } else if (m_shapeToDelete && m_isShapeOwnedByList) {
        // 如果 m_isShapeOwnedByList 为 true，表示图形仍在 ArtboardView 的 shapesList 中
        // (例如，命令被创建但从未执行，或者执行后被成功 undo 了，图形已恢复到列表)。
        // 这种情况下，命令的析构函数不应该 delete m_shapeToDelete，
        // 因为图形的生命周期此时由 ArtboardView 的列表或其清理机制（如 clearAllShapes）管理。
        qDebug() << "DeleteShapeCommand Destructor: Shape at" << (void*)m_shapeToDelete << "NOT deleted (still owned by list).";
    }
}

/// @brief 执行“删除图形”命令。
/// 此方法将 m_shapeToDelete 从 ArtboardView 的图形列表中移除，并更新视图。
void DeleteShapeCommand::execute()
{
    // 安全检查
    if (!m_artboardView || !m_shapeToDelete) {
        qWarning("DeleteShapeCommand::execute() - ArtboardView or Shape to delete is null.");
        return;
    }

    // 1. 从 ArtboardView 的 shapesList 中移除该图形。
    //    QVector::removeOne() 会移除第一个与 m_shapeToDelete 指针匹配的元素。
    //    这假设 shapesList 中没有重复的指针指向同一个对象。
    bool removed = m_artboardView->shapesList.removeOne(m_shapeToDelete); // 直接通过友元访问 shapesList

    if (removed) {
        // 2. 更新所有权标志：图形已从视图列表移除，其所有权现在由本命令对象暂时“接管”
        //    （主要指在命令被销毁且未撤销时，由本命令负责 delete）。
        m_isShapeOwnedByList = false;

        // 3. 请求 ArtboardView 重绘以反映图形的移除。
        m_artboardView->update();
        qDebug() << "DeleteShapeCommand: Executed - Shape at" << (void*)m_shapeToDelete << "removed from view. Original index was:" << m_originalIndex;
    } else {
        // 如果图形在列表中没找到，可能意味着状态不一致或逻辑错误（例如，尝试删除一个已被删除的图形）。
        qDebug() << "DeleteShapeCommand::execute() - Shape at" << (void*)m_shapeToDelete << "was not found in view's list to remove.";
        // 即使没找到，也应该将 m_isShapeOwnedByList 设为 false，因为命令的意图是确保它不在列表中。
        m_isShapeOwnedByList = false;
    }
}

/// @brief 撤销“删除图形”命令（即恢复图形）。
/// 此方法将 m_shapeToDelete 重新插入到 ArtboardView 图形列表的原始位置 (m_originalIndex)，并更新视图。
void DeleteShapeCommand::undo()
{
    // 安全检查
    if (!m_artboardView || !m_shapeToDelete) {
        qWarning("DeleteShapeCommand::undo() - ArtboardView or Shape to restore is null.");
        return;
    }

    // 1. 检查原始索引的有效性，以防止越界插入。
    //    m_originalIndex 应该在 [0, shapesList.size()] 范围内。
    //    (size() 表示可以插入到末尾)
    if (m_originalIndex >= 0 && m_originalIndex <= m_artboardView->shapesList.size()) {
        // 2. 将图形对象重新插入到 shapesList 的原始索引位置。
        m_artboardView->shapesList.insert(m_originalIndex, m_shapeToDelete); // 通过友元访问

        // 3. 更新所有权标志：图形已恢复到视图列表，其生命周期主要由视图列表管理。
        m_isShapeOwnedByList = true;

        // 4. 请求 ArtboardView 重绘以显示恢复的图形。
        m_artboardView->update();
        qDebug() << "DeleteShapeCommand: Undone - Shape at" << (void*)m_shapeToDelete << "re-inserted into view at index:" << m_originalIndex;
    } else {
        // 如果原始索引无效（例如，列表大小发生了意外的巨大变化），这是一个潜在的问题。
        // 可以考虑一种容错策略，比如追加到列表末尾，或者更严格地报错。
        qWarning() << "DeleteShapeCommand::undo() - Invalid original index (" << m_originalIndex
                   << ") for restoring shape. List size is" << m_artboardView->shapesList.size()
                   << ". Shape at" << (void*)m_shapeToDelete << "was not restored to its original position.";
        // 作为一种备选的、可能不完全正确的恢复方式，可以尝试追加到末尾，
        // 但这会破坏图形的原始绘制顺序。
        // m_artboardView->shapesList.append(m_shapeToDelete);
        // m_isShapeOwnedByList = true;
        // m_artboardView->update();
        // 目前，如果索引无效，我们选择不恢复，并打印警告，以暴露潜在问题。
    }
}
