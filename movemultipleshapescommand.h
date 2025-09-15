#ifndef MOVEMULTIPLESHAPESCOMMAND_H
#define MOVEMULTIPLESHAPESCOMMAND_H

#include "abstractcommand.h"
#include <QList>
#include <QPoint>

class AbstractShape;
class ArtboardView;

// 宏命令，用于将多个图形的移动打包成一个单一的可撤销操作
class MoveMultipleShapesCommand : public AbstractCommand
{
public:
    MoveMultipleShapesCommand(const QList<AbstractShape*> &shapes, const QPoint &offset, ArtboardView *view);
    ~MoveMultipleShapesCommand() override;

    void execute() override;
    void undo() override;

private:
    QList<AbstractShape*> m_shapes;
    QPoint m_offset;
    ArtboardView *m_view;
};

#endif // MOVEMULTIPLESHAPESCOMMAND_H
