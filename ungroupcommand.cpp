#include "ungroupcommand.h"
#include "artboardview.h"
#include "groupshape.h"

UngroupCommand::UngroupCommand(GroupShape *group, ArtboardView *view)
    : m_view(view), m_group(group)
{
    m_originalGroupIndex = m_view->shapesList.indexOf(m_group);
}

UngroupCommand::~UngroupCommand()
{
    // 如果组对象已在视图中（例如命令被撤销），则析构函数不应删除它。
    // 如果组对象不在视图中（例如命令执行后被销毁），则它应该被删除。
    if (m_group && m_view && !m_view->shapesList.contains(m_group)) {
        // 在这种情况下，子图形已经被移出，GroupShape析构时不会重复删除
        delete m_group;
    }
}

void UngroupCommand::execute()
{
    if (!m_view || !m_group || m_originalGroupIndex == -1) return;

    // 从视图中移除组
    m_view->shapesList.removeOne(m_group);

    // 获取子图形列表的所有权
    m_children = m_group->takeChildren();

    // 将子图形添加到视图中
    for (AbstractShape* child : m_children) {
        m_view->shapesList.append(child);
    }

    // 更新选择
    m_view->m_selectedShapes.clear();
    m_view->m_selectedShapes = m_children;

    m_view->update();
}


void UngroupCommand::undo()
{
    if (!m_view || m_children.isEmpty() || !m_group) return;

    // 1. 从视图中移除刚刚被取消编组的子图形
    for (AbstractShape* child : m_children) {
        m_view->shapesList.removeOne(child);
    }

    // 2. [ 关键修正 ]
    // 将子图形重新添加回原有的组对象中，而不是销毁并重建它
    m_group->addChildren(m_children);
    m_children.clear(); // 此时所有权已安全交还给组

    // 3. 将恢复了内容的组对象重新插入其原始位置
    m_view->shapesList.insert(m_originalGroupIndex, m_group);

    // 4. 恢复选择
    m_view->m_selectedShapes.clear();
    m_view->m_selectedShapes.append(m_group);

    m_view->update();
}
