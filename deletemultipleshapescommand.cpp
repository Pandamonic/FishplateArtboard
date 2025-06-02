// ---------------------------------------------------------------------------
// 描述: DeleteMultipleShapesCommand 类的实现文件。
//       包含宏命令的构造、析构、批量执行和批量撤销的逻辑。
// ---------------------------------------------------------------------------

#include "deletemultipleshapescommand.h"
#include "deleteshapecommand.h" // 需要 DeleteShapeCommand 的完整定义来操作它
#include <QDebug>               // 用于调试输出

/// @brief DeleteMultipleShapesCommand 构造函数的实现。
/// @param commands 一个 QList，包含指向要批量处理的 DeleteShapeCommand 对象的指针。
///                 宏命令将接管这些子命令的所有权。
DeleteMultipleShapesCommand::DeleteMultipleShapesCommand(const QList<DeleteShapeCommand*> &commands)
    : m_deleteCommands(commands) // 初始化成员列表，直接使用传入的 QList (浅拷贝指针)
{
    qDebug() << "DeleteMultipleShapesCommand created, containing" << m_deleteCommands.size() << "sub-commands.";
}

/// @brief DeleteMultipleShapesCommand 析构函数的实现。
/// 负责清理并释放其内部 `m_deleteCommands` 列表中存储的所有 `DeleteShapeCommand` 对象。
DeleteMultipleShapesCommand::~DeleteMultipleShapesCommand()
{
    qDebug() << "DeleteMultipleShapesCommand: Deleting" << m_deleteCommands.size() << "sub-commands from macro command.";
    // qDeleteAll 是 Qt 提供的便捷函数，它会遍历容器中的每个指针，并对其调用 delete。
    qDeleteAll(m_deleteCommands);
    m_deleteCommands.clear(); // 清空列表本身（移除所有现在是空悬的指针）
}

/// @brief 执行宏命令，即依次执行其包含的所有单个 DeleteShapeCommand 的 execute() 方法。
/// 子命令的执行顺序通常是它们被添加到列表时的顺序（例如，按图形在画布上被选中的顺序，或者按索引从高到低）。
void DeleteMultipleShapesCommand::execute()
{
    qDebug() << "DeleteMultipleShapesCommand: Executing all" << m_deleteCommands.size() << "sub-delete-commands.";
    // 正序执行所有子删除命令。
    // 每个子 DeleteShapeCommand::execute() 会从 ArtboardView::shapesList 中移除图形。
    // 如果子命令是按原始索引从高到低排列的，这里的正序执行是正确的。
    for (DeleteShapeCommand *cmd : m_deleteCommands) { // 使用范围 for 循环
        if (cmd) { // 安全检查
            cmd->execute();
        }
    }
    // 注意：ArtboardView::update() 应该由每个子命令的 execute() 调用，
    // 或者由最后调用此宏命令的 executeCommand() 在所有操作完成后统一调用一次。
    // 我们当前的 DeleteShapeCommand::execute() 会调用 ArtboardView::update()。
}

/// @brief 撤销宏命令，即依次撤销其包含的所有单个 DeleteShapeCommand 的 undo() 方法。
/// 为了正确恢复图形到它们在列表中的原始位置（特别是如果删除操作影响了后续图形的索引），
/// 子命令的撤销顺序通常需要与它们被执行（或添加到宏命令列表时考虑的原始索引顺序）的顺序相反。
void DeleteMultipleShapesCommand::undo()
{
    qDebug() << "DeleteMultipleShapesCommand: Undoing all" << m_deleteCommands.size() << "sub-delete-commands.";
    // 假设 m_deleteCommands 列表中的命令是按照图形在 shapesList 中原始索引从高到低排列的
    // 那么在 undo 时，我们应该先恢复原始索引较低的图形，再恢复原始索引较高的图形，
    // 以确保它们能被正确地插入回原来的位置，而不受其他恢复操作的影响。
    // 因此，我们需要从 m_deleteCommands 列表的末尾向前遍历。
    for (int i = m_deleteCommands.size() - 1; i >= 0; --i) {
        DeleteShapeCommand *cmd = m_deleteCommands.at(i);
        if (cmd) {
            cmd->undo(); // 调用子命令的 undo，它会将图形插回原始索引位置
        }
    }
}
