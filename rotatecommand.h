// ----------------- rotatecommand.h (请完整替换此文件内容) -----------------
#ifndef ROTATECOMMAND_H
#define ROTATECOMMAND_H

#include <QtGlobal> // 包含此文件以定义 qreal

#include "abstractcommand.h"

// 前向声明
class AbstractShape;
class ArtboardView;

class RotateCommand : public AbstractCommand
{
public:
    // 构造函数的声明，需要与.cpp文件中的实现完全匹配
    RotateCommand(AbstractShape *shape, ArtboardView *view, qreal oldAngle, qreal newAngle);
    ~RotateCommand() override {}

    void execute() override;
    void undo() override;

private:
    // 这里是所有成员变量的声明，C++代码将在这里找到它们
    AbstractShape *m_shape;
    ArtboardView *m_view;
    qreal m_oldAngle;
    qreal m_newAngle;
};

#endif // ROTATECOMMAND_H
