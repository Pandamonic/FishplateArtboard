#ifndef RESIZECOMMAND_H
#define RESIZECOMMAND_H

#include "abstractcommand.h"
#include <QRect>

class AbstractShape;
class ArtboardView; // 前向声明

class ResizeCommand : public AbstractCommand
{
public:
    // 我们需要 ArtboardView* 以便在命令执行后调用 update()
    ResizeCommand(AbstractShape *shape, ArtboardView *view, const QRect &oldRect, const QRect &newRect);
    ~ResizeCommand() override {}

    void execute() override;
    void undo() override;

private:
    AbstractShape *m_shape;
    ArtboardView *m_view;
    QRect m_oldRect;
    QRect m_newRect;
};

#endif // RESIZECOMMAND_H
