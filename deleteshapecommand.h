#ifndef DELETESHAPECOMMAND_H
#define DELETESHAPECOMMAND_H

// ---------------------------------------------------------------------------
// 文件名: deleteshapecommand.h
// 项目名: Fishplate Artboard
// 描述: 定义了 DeleteShapeCommand 类，这是一个具体的命令类，
//       用于封装从画布上删除一个指定图形对象的操作。
//       它继承自 AbstractCommand 并实现了 execute() 和 undo() 方法。
// ---------------------------------------------------------------------------

#include "abstractcommand.h" // 包含抽象命令基类的头文件
#include "abstractshape.h"   // 命令操作的是 AbstractShape 类型的对象

// 向前声明 ArtboardView 类，以避免在头文件中直接 #include "artboardview.h"
class ArtboardView;

/// @brief DeleteShapeCommand 类封装了从 ArtboardView 删除一个图形对象的操作。
/// 这个命令可以被执行（删除图形）和撤销（恢复图形）。
/// 它负责管理被删除图形对象 (AbstractShape) 的部分生命周期，
/// 特别是在命令被执行后（图形从视图移除）且命令本身最终被销毁时，需要释放图形内存。
class DeleteShapeCommand : public AbstractCommand
{
public:
    /// @brief DeleteShapeCommand 的构造函数。
    /// @param shapeToDelete 指向要从视图中删除的 AbstractShape 对象的指针。
    ///                      此命令在执行删除操作后，会“拥有”这个图形对象，直到它被撤销或命令被销毁。
    /// @param view 指向 ArtboardView 实例的指针，命令将通过它来操作图形列表。
    /// @param index 图形 `shapeToDelete` 在 `view` 的 `shapesList` 中的原始索引。
    ///              这个索引非常重要，用于在 `undo()` 操作时将图形恢复到其原始位置。
    DeleteShapeCommand(AbstractShape *shapeToDelete, ArtboardView *view, int index);

    /// @brief DeleteShapeCommand 的析构函数。
    /// 负责在命令对象被销毁时，有条件地释放其持有的 `m_shapeToDelete` 图形对象。
    /// 释放条件取决于 `m_shapeToDelete` 当前是否已通过 `undo()` 操作恢复到视图的列表中
    /// (通过 `m_isShapeOwnedByList` 标志判断)。
    ~DeleteShapeCommand() override;

    // --- 从 AbstractCommand 继承并重写的虚函数 ---

    /// @brief 执行“删除图形”的操作。
    /// 将命令持有的图形对象 (`m_shapeToDelete`) 从 ArtboardView 的内部图形列表中移除，
    /// 并更新视图。同时标记图形已从视图列表移除，其所有权由命令对象暂时接管。
    void execute() override;

    /// @brief 撤销“删除图形”的操作（即恢复图形）。
    /// 将命令持有的图形对象 (`m_shapeToDelete`) 重新插入到 ArtboardView 内部图形列表的
    /// 原始位置 (`m_originalIndex`)，并更新视图。同时标记图形的所有权回归到视图列表。
    void undo() override;


    // --- (可选) 调试辅助方法 ---
    /// @brief 获取此命令关联的、被删除（或待恢复）的图形对象指针。
    /// @return 指向 AbstractShape 对象的指针。
    AbstractShape* getShapeDeletedForDebug() const { return m_shapeToDelete; }
    /// @brief 获取图形被删除前在列表中的原始索引。
    /// @return 整数索引值。
    int getOriginalIndexForDebug() const { return m_originalIndex; }


private:
    AbstractShape *m_shapeToDelete; ///< 命令所持有的、将被删除或已被删除/待恢复的图形对象。
    ArtboardView *m_artboardView;   ///< 指向 ArtboardView 实例，用于执行操作。
    int m_originalIndex;            ///< 图形对象在 `shapesList` 中的原始索引，用于 `undo` 操作。
    bool m_isShapeOwnedByList;      ///< 标志 `m_shapeToDelete` 指向的图形对象当前是否在 ArtboardView 的
        ///< `shapesList` 中并由其主要管理。
        ///< true = 图形在列表中 (例如命令刚创建或被 undo 后)；
        ///< false = 图形不在列表中 (例如命令被 execute 后)，此时命令的析构函数负责 delete 它。
};

#endif // DELETESHAPECOMMAND_H
