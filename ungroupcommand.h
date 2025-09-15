#ifndef UNGROUPCOMMAND_H
#define UNGROUPCOMMAND_H

#include "abstractcommand.h"
#include <QList>

class AbstractShape;
class GroupShape;
class ArtboardView;

class UngroupCommand : public AbstractCommand
{
public:
    UngroupCommand(GroupShape *group, ArtboardView *view);
    ~UngroupCommand() override;

    void execute() override;
    void undo() override;

private:
    ArtboardView *m_view;
    GroupShape *m_group; // 要取消编组的组对象
    QList<AbstractShape*> m_children; // 用于撤销时恢复
    int m_originalGroupIndex;
};

#endif // UNGROUPCOMMAND_H
