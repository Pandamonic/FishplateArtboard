#ifndef ADDSHAPECOMMAND_H
#define ADDSHAPECOMMAND_H

// ---------------------------------------------------------------------------
// 描述: 定义了 AddShapeCommand 类，用于封装"将一个新的图形对象添加到画布上"的操作。
//       继承自 AbstractCommand 并实现了 execute() 和 undo() 方法。
// ---------------------------------------------------------------------------

#include "abstractcommand.h"
#include "abstractshape.h"   // 命令操作的是 AbstractShape 类型的对象

// AddShapeCommand 的实现文件 (addshapecommand.cpp) 将会包含 "artboardview.h" 的完整定义。
class ArtboardView;

/// @brief AddShapeCommand 类封装了向 ArtboardView 添加一个新图形的操作。
/// 这个命令可以被执行（添加图形）和撤销（移除图形）。
/// 它负责管理被添加图形对象 (AbstractShape) 的部分生命周期，
/// 特别是在命令被撤销或命令本身被销毁时。
class AddShapeCommand : public AbstractCommand
{
public:
    /// @brief AddShapeCommand 的构造函数。
    /// @param shape 指向要添加到视图的 AbstractShape 对象的指针。此命令在创建时会获得对这个 shape 对象的部分所有权，
    ///              并在特定情况下负责释放其内存。
    /// @param view 指向 ArtboardView 实例的指针，命令将通过它来操作图形列表。
    AddShapeCommand(AbstractShape *shape, ArtboardView *view);

    /// @brief AddShapeCommand 的析构函数。
    /// 负责在命令对象被销毁时，有条件地释放其持有的 m_shapeToAdd 图形对象。
    /// 释放条件取决于 m_shapeToAdd 当前是否已被 ArtboardView 的图形列表所“拥有”
    ~AddShapeCommand() override;

    // --- 从 AbstractCommand 继承并重写的虚函数 ---

    /// @brief 执行添加图形的操作。
    /// 将命令持有的图形对象 (m_shapeToAdd) 添加到 ArtboardView 的内部图形列表中，并更新视图。同时标记图形现在由视图管理。
    void execute() override;

    /// @brief 撤销添加图形的操作。
    /// 将命令持有的图形对象 (m_shapeToAdd) 从 ArtboardView 的内部图形列表中移除，并更新视图。同时标记图形的所有权回归到命令对象。
    void undo() override;

    /// @brief 获取此命令关联的图形对象指针。
    /// @return 指向 AbstractShape 对象的指针。
    AbstractShape* getShapeForDebug() const { return m_shapeToAdd; }

private:
    AbstractShape *m_shapeToAdd;  ///< 命令所持有的、将要被添加或已被添加/移除的图形对象。
    ArtboardView *m_artboardView; ///< 指向 ArtboardView 实例，用于执行操作。
    bool m_isShapeOwnedByView;    ///< 标志 m_shapeToAdd 指向的图形对象当前是否在 ArtboardView 的
        ///< shapesList 中并由其主要管理。
        ///< true = 图形在列表中，其生命周期主要由 ArtboardView 的列表清理逻辑负责；
        ///< false = 图形不在列表中（例如被 undo 之后），如果命令被销毁，则命令的析构函数负责 delete 它。
};

#endif // ADDSHAPECOMMAND_H
