// ---------------------------------------------------------------------------
// 描述: AddShapeCommand 类的实现文件。
//       包含了构造函数、析构函数以及 execute() 和 undo() 方法的具体实现。
// ---------------------------------------------------------------------------

#include "addshapecommand.h"
#include "artboardview.h" // 需要 ArtboardView 的完整定义，以便调用其方法或访问其成员(如果设为友元)
#include <QDebug>         // 用于调试输出

/// @brief AddShapeCommand 构造函数的实现。
/// @param shape 指向要添加的 AbstractShape 对象的指针。命令对象获得此图形的部分所有权。
/// @param view 指向 ArtboardView 实例的指针，命令将通过它与画布交互。
AddShapeCommand::AddShapeCommand(AbstractShape *shape, ArtboardView *view)
    : m_shapeToAdd(shape),        // 初始化，存储要添加的图形对象的指针
    m_artboardView(view),         // 初始化，存储 ArtboardView 的指针
    m_isShapeOwnedByView(false)   // 关键初始化：命令刚创建时，图形尚未被添加到视图的列表中，
    // 因此视图尚未“拥有”或“管理”它，此标志设为 false。
    // 命令对象此时是 m_shapeToAdd 的主要管理者。
{
    // qDebug() << "AddShapeCommand CONSTRUCTOR: Command Addr:" << this << "Shape Addr:" << m_shapeToAdd;
}

/// @brief AddShapeCommand 析构函数的实现。
/// 负责在命令对象本身被销毁时，正确处理其持有的图形对象 m_shapeToAdd 的内存。
AddShapeCommand::~AddShapeCommand()
{
    // 如果 m_shapeToAdd 指针有效（非空），并且 m_isShapeOwnedByView 标志为 false
    // (意味着图形当前不在 ArtboardView 的 shapesList 中，比如命令被 undo 了，或者从未 execute)，
    // 那么此 AddShapeCommand 对象在被销毁时，就有责任 delete 它所持有的 m_shapeToAdd，以防止内存泄漏。
    if (m_shapeToAdd && !m_isShapeOwnedByView) {
        delete m_shapeToAdd;
        m_shapeToAdd = nullptr;
        qDebug() << "AddShapeCommand Destructor: Shape at" << (void*)m_shapeToAdd << "deleted (was not owned by view).";
    } else if (m_shapeToAdd && m_isShapeOwnedByView) {
        // 如果 m_isShapeOwnedByView 为 true，表示图形仍在 ArtboardView 的 shapesList 中，
        // 它的生命周期将由 ArtboardView（例如在其析构函数或 clearAllShapes 方法中）管理。
        // 这种情况下，此命令的析构函数不应该 delete m_shapeToAdd，以避免重复删除。
        qDebug() << "AddShapeCommand Destructor: Shape at" << (void*)m_shapeToAdd << "NOT deleted (owned by view).";
    }
    // 如果 m_shapeToAdd 为 nullptr (例如在 execute 成功后被 ArtboardView 接管了指针所有权)，则无需操作。
}

/// @brief 执行“添加图形”命令。
/// 此方法将 m_shapeToAdd 添加到 ArtboardView 的图形列表中，并更新视图。
void AddShapeCommand::execute()
{
    if (!m_artboardView || !m_shapeToAdd) { // 特判
        qWarning("AddShapeCommand::execute() - ArtboardView or Shape to add is null.");
        return;
    }

    // 1. 将图形添加到 ArtboardView 的 shapesList 中。
    m_artboardView->shapesList.append(m_shapeToAdd);

    // 2. 更新所有权标志：图形现在被视图的列表所管理。
    m_isShapeOwnedByView = true;

    // 3. 请求 ArtboardView 重绘以显示新添加的图形。
    m_artboardView->update();

    qDebug() << "AddShapeCommand: Executed - Shape at" << (void*)m_shapeToAdd << "added to view.";
}

/// @brief 撤销“添加图形”命令。
/// 此方法将 m_shapeToAdd 从 ArtboardView 的图形列表中移除，并更新视图。
void AddShapeCommand::undo()
{
    // 安全检查
    if (!m_artboardView || !m_shapeToAdd) {
        qWarning("AddShapeCommand::undo() - ArtboardView or Shape to remove is null.");
        return;
    }

    // 1. 从 ArtboardView 的 shapesList 中移除该图形。
    //    QVector::removeAll() 会移除所有指向 m_shapeToAdd 内存地址的指针。
    int removedCount = m_artboardView->shapesList.removeAll(m_shapeToAdd);

    if (removedCount > 0) {
        // 2. 更新所有权标志：图形已从视图列表移除，命令重新完全“拥有”它。
        m_isShapeOwnedByView = false;

        // 3. 请求 ArtboardView 重绘以反映图形的移除。
        m_artboardView->update();
        qDebug() << "AddShapeCommand: Undone - Shape at" << (void*)m_shapeToAdd << "removed from view.";
    } else {
        // 如果图形在列表中没找到，可能意味着状态不一致或逻辑错误。
        qDebug() << "AddShapeCommand::undo() - Shape at" << (void*)m_shapeToAdd << "was not found in view's list to remove.";
        // 即使没找到，也应该将 m_isShapeOwnedByView 设为 false。
        m_isShapeOwnedByView = false;
    }
}
