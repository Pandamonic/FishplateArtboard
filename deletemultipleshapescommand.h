#ifndef DELETEMULTIPLESHAPESCOMMAND_H
#define DELETEMULTIPLESHAPESCOMMAND_H

// ---------------------------------------------------------------------------
// 描述: 定义了 DeleteMultipleShapesCommand 类，这是一个宏命令，
//       用于封装一次性删除多个图形对象的操作。
//       它通过持有一系列单独的 DeleteShapeCommand 对象来实现批量删除和恢复。
//       继承自 AbstractCommand。
// ---------------------------------------------------------------------------

#include "abstractcommand.h"    // 包含抽象命令基类的头文件
#include <QList>                // 用于存储子命令的列表

// 向前声明 DeleteShapeCommand，因为我们在这里只使用了它的指针类型。
// 完整的 DeleteShapeCommand 定义在 "deleteshapecommand.h" 中，
// 将在 .cpp 文件中包含。
class DeleteShapeCommand;

/// @brief DeleteMultipleShapesCommand 类是一个宏命令，用于将多个图形的删除操作
///        组合成一个单一的可撤销/重做单元。
///
/// 当用户执行一个可能导致多个图形被删除的操作（例如，拖动式笔画橡皮擦）时，
/// 可以创建一个此类的实例，并将每个单独的删除操作（封装为 DeleteShapeCommand）
/// 添加到此宏命令中。执行此宏命令会依次执行所有子删除命令；撤销此宏命令会
/// 依次（通常以相反顺序）撤销所有子删除命令。
///
/// 此命令对象拥有其包含的所有子命令 (DeleteShapeCommand) 对象的所有权。
class DeleteMultipleShapesCommand : public AbstractCommand
{
public:
    /// @brief DeleteMultipleShapesCommand 的构造函数。
    /// @param commands 一个 QList，包含指向要批量执行的 DeleteShapeCommand 对象的指针。
    ///                 此宏命令将获得这些传入的 DeleteShapeCommand 对象的所有权，
    ///                 并在自身被销毁时负责释放它们。
    explicit DeleteMultipleShapesCommand(const QList<DeleteShapeCommand*> &commands);

    /// @brief DeleteMultipleShapesCommand 的析构函数。
    /// 负责删除并释放在构造时获得的所有子命令 (DeleteShapeCommand 对象)。
    ~DeleteMultipleShapesCommand() override;

    // --- 从 AbstractCommand 继承并重写的虚函数 ---

    /// @brief 执行“批量删除图形”的操作。
    /// 此方法会遍历内部存储的 DeleteShapeCommand 列表，并依次调用每个子命令的 execute() 方法。
    void execute() override;

    /// @brief 撤销“批量删除图形”的操作（即恢复所有被删除的图形）。
    /// 此方法会遍历内部存储的 DeleteShapeCommand 列表（通常以与执行时相反的顺序），
    /// 并依次调用每个子命令的 undo() 方法。
    void undo() override;

private:
    QList<DeleteShapeCommand*> m_deleteCommands; ///< 存储一系列指向单个 DeleteShapeCommand 对象的指针。
        ///< 此宏命令拥有这些子命令对象。
};

#endif // DELETEMULTIPLESHAPESCOMMAND_H
