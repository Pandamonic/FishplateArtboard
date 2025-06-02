#ifndef CLEARALLCOMMAND_H
#define CLEARALLCOMMAND_H

// ---------------------------------------------------------------------------
// 描述: 定义了 ClearAllCommand 类，这是一个具体的命令类，
//       用于封装清空画布上所有已绘制图形的操作。
//       它会备份被清空的图形列表，以便能够撤销此操作。
//       继承自 AbstractCommand。
// ---------------------------------------------------------------------------

#include "abstractcommand.h" // 包含抽象命令基类的头文件
#include <QVector>           // 用于存储被清空图形的列表

// 向前声明 AbstractShape 和 ArtboardView 类，以减少头文件依赖
class AbstractShape;
class ArtboardView;

/// @brief ClearAllCommand 类封装了清空 ArtboardView 画布上所有图形的操作。
///
/// 当用户执行“清空画布”功能时，会创建此命令的实例。
/// - execute(): 将 ArtboardView 当前的图形列表备份到命令内部，然后清空视图的图形列表。
/// - undo(): 将备份的图形列表恢复到 ArtboardView 中。
///
/// 此命令对象拥有在其内部 `m_clearedShapes` 列表中存储的图形对象的所有权，
/// 特别是在命令被执行后（图形从视图的主列表移走）且命令最终被销毁（而未被撤销）时，
/// 或者在命令被撤销后（图形已移回主列表）命令被销毁时，需要正确管理这些图形的内存。
class ClearAllCommand : public AbstractCommand
{
public:
    /// @brief ClearAllCommand 的构造函数。
    /// @param view 指向 ArtboardView 实例的指针，命令将通过它来操作图形列表和刷新视图。
    explicit ClearAllCommand(ArtboardView *view);

    /// @brief ClearAllCommand 的析构函数。
    /// 负责删除并释放在 `execute()` 操作中备份到 `m_clearedShapes` 列表中的所有图形对象，
    /// 前提是这些图形没有通过 `undo()` 操作恢复到 ArtboardView 的主列表中。
    ~ClearAllCommand() override;

    // --- 从 AbstractCommand 继承并重写的虚函数 ---

    /// @brief 执行“清空所有图形”的操作。
    /// 1. 将 ArtboardView 当前 `shapesList` 中的所有图形指针移动到本命令的 `m_clearedShapes` 列表中进行备份。
    /// 2. 清空 ArtboardView 的 `shapesList`。
    /// 3. 更新视图。
    void execute() override;

    /// @brief 撤销“清空所有图形”的操作（即恢复所有图形）。
    /// 1. 将本命令备份在 `m_clearedShapes` 列表中的所有图形指针移回到 ArtboardView 的 `shapesList` 中。
    /// 2. 清空本命令的 `m_clearedShapes` 列表 (因为所有权已转移回视图)。
    /// 3. 更新视图。
    void undo() override;

private:
    ArtboardView *m_artboardView;                 ///< 指向 ArtboardView 实例。
    QVector<AbstractShape*> m_clearedShapes;      ///< 用于存储在执行清空操作时，从 ArtboardView 的
        ///< shapesList 中备份出来的图形对象的指针列表。
        ///< 此命令对象在特定情况下“拥有”这些图形的内存。
};

#endif // CLEARALLCOMMAND_H
