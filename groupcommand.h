#ifndef GROUPCOMMAND_H
#define GROUPCOMMAND_H

#include "abstractcommand.h"
#include <QList>

class AbstractShape;
class ArtboardView;

class GroupCommand : public AbstractCommand
{
public:
    GroupCommand(const QList<AbstractShape*> &shapesToGroup, ArtboardView *view);
    ~GroupCommand() override;

    void execute() override;
    void undo() override;

private:
    ArtboardView *m_view;
    QList<AbstractShape*> m_shapesToGroup; // 用于撤销时恢复
    // Qt 6.5及以上可以使用QList<qsizetype>，否则用QList<int>
    QList<int> m_originalIndices; // 保存原始索引以正确撤销
    AbstractShape *m_groupShape; // 创建的组对象
};

#endif // GROUPCOMMAND_H
