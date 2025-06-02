#ifndef MOVESHAPECOMMAND_H
#define MOVESHAPECOMMAND_H

// ---------------------------------------------------------------------------
// 描述: 定义了 MoveShapeCommand 类，这是一个具体的命令类，
//       用于封装移动画布上一个已存在图形对象的操作。
//       它记录了图形移动的偏移量，以便能够执行移动和撤销移动。
//       继承自 AbstractCommand。
// ---------------------------------------------------------------------------

#include "abstractcommand.h" // 包含抽象命令基类的头文件
#include "abstractshape.h"   // 命令操作的是 AbstractShape 类型的对象
#include <QPoint>            // 需要 QPoint 来表示移动的偏移量

// 向前声明 ArtboardView 类，以减少头文件依赖
class ArtboardView;

/// @brief MoveShapeCommand 类封装了移动单个图形对象的操作。
///
/// 当用户通过“选择”工具拖动一个图形到新位置后，会创建一个此命令的实例。
/// 它存储了被移动的图形对象以及本次移动的净偏移量。
/// - execute(): 将图形按偏移量移动（主要用于Redo，因为初次拖动已实时移动）。
/// - undo(): 将图形按相反的偏移量移回，恢复到移动前的位置。
///
/// 注意：此命令不拥有被移动的图形对象 (m_shapeMoved) 的所有权；
/// 图形对象的生命周期由 ArtboardView 的 shapesList 或其他创建型命令管理。
class MoveShapeCommand : public AbstractCommand
{
public:
    /// @brief MoveShapeCommand 的构造函数。
    /// @param shapeToMove 指向要被移动的 AbstractShape 对象的指针。
    /// @param moveOffset 本次移动操作的净偏移量 (QPoint)。这是从图形原始位置到新位置的向量。
    /// @param view 指向 ArtboardView 实例的指针，如果命令需要直接与视图交互（例如调用 update()）。
    ///             在我们的实现中，图形的 moveBy() 之后，视图通常会通过其他方式（如命令执行后的统一update）更新。
    MoveShapeCommand(AbstractShape *shapeToMove, const QPoint &moveOffset, ArtboardView *view);

    /// @brief MoveShapeCommand 的析构函数。
    /// 由于此命令不拥有 m_shapeMoved 图形对象，因此析构函数通常为空，
    /// 不需要释放 m_shapeMoved 的内存。
    ~MoveShapeCommand() override {}

    // --- 从 AbstractCommand 继承并重写的虚函数 ---

    /// @brief 执行“移动图形”的操作。
    /// 将 m_shapeMoved 图形对象按照构造时传入的 m_offset 进行移动。
    /// 此方法主要在“重做 (Redo)”操作时被调用，因为初次拖动时图形可能已被实时移动到目标位置。
    /// （我们的实现策略是：mouseMove实时移动，mouseRelease时先将图形移回原位，再让此命令的execute将其移到新位）
    void execute() override;

    /// @brief 撤销“移动图形”的操作。
    /// 将 m_shapeMoved 图形对象按照 m_offset 的相反方向移动，使其恢复到移动前的位置。
    void undo() override;

private:
    AbstractShape *m_shapeMoved;  ///< 指向被移动的图形对象。命令不拥有此对象。
    QPoint m_offset;              ///< 本次移动操作的净偏移量 (从原始位置到新位置的向量)。
    ArtboardView *m_artboardView; ///< 指向 ArtboardView 实例的指针，用于可能的视图更新。
};

#endif // MOVESHAPECOMMAND_H
