#include "groupcommand.h"
#include "artboardview.h"
#include "groupshape.h"
#include <algorithm> // for std::sort

GroupCommand::GroupCommand(const QList<AbstractShape*> &shapesToGroup, ArtboardView *view)
    : m_view(view), m_shapesToGroup(shapesToGroup), m_groupShape(nullptr)
{
    // 保存原始索引，用于撤销
    for(AbstractShape* shape : m_shapesToGroup) {
        m_originalIndices.append(m_view->shapesList.indexOf(shape));
    }
}

GroupCommand::~GroupCommand()
{
    // 如果组对象没有被添加到视图中（例如，命令在撤销后被销毁），
    // 则需要手动删除它，以避免内存泄漏。
    if (m_groupShape && m_view && !m_view->shapesList.contains(m_groupShape)) {
        delete m_groupShape;
    }
}

void GroupCommand::execute()
{
    if (!m_view || m_shapesToGroup.count() < 2) return;

    // 为了安全地移除，我们从高索引到低索引进行
    QList<int> sortedIndices = m_originalIndices;
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());
    for (int index : sortedIndices) {
        m_view->shapesList.removeAt(index);
    }

    // [ 关键修正 ]
    // 如果是第一次执行，则创建组对象；如果是重做，则将图形重新添加到已存在的组对象中
    if (!m_groupShape) {
        m_groupShape = new GroupShape(m_shapesToGroup);
    } else {
        static_cast<GroupShape*>(m_groupShape)->addChildren(m_shapesToGroup);
    }

    m_view->shapesList.append(m_groupShape);

    // 更新选择
    m_view->m_selectedShapes.clear();
    m_view->m_selectedShapes.append(m_groupShape);

    m_view->update();
}

// ----------------- groupcommand.cpp (请完整替换此函数) -----------------
void GroupCommand::undo()
{
    if (!m_view || !m_groupShape) return;

    // 1. 从视图中移除组对象
    m_view->shapesList.removeOne(m_groupShape);

    // 2. [ 关键修正 ]
    // 让组对象移交其子图形的所有权。
    // 我们必须从组对象本身获取子对象，而不是依赖备份列表，
    // 以确保状态的绝对一致性。
    QList<AbstractShape*> children = static_cast<GroupShape*>(m_groupShape)->takeChildren();

    // 3. 按原始索引恢复子图形
    // 注意：我们必须确保恢复的图形列表(children)和索引列表(m_originalIndices)匹配
    for (int i = 0; i < children.count(); ++i) {
        // 找到这个子图形对应的原始索引
        int originalShapeIndex = m_shapesToGroup.indexOf(children.at(i));
        int indexToInsert = m_originalIndices.at(originalShapeIndex);
        m_view->shapesList.insert(indexToInsert, children.at(i));
    }

    // 4. 恢复选择状态为原来的多个图形
    m_view->m_selectedShapes.clear();
    m_view->m_selectedShapes = children; // 使用从组里拿出来的、最新的子图形列表

    m_view->update();
}
