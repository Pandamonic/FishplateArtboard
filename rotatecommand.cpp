#include "rotatecommand.h"
#include "abstractshape.h"
#include "artboardview.h"

RotateCommand::RotateCommand(AbstractShape *shape, ArtboardView *view, qreal oldAngle, qreal newAngle)
    : m_shape(shape),
    m_view(view),
    m_oldAngle(oldAngle),
    m_newAngle(newAngle)
{
}

// 执行命令 = 将图形设置为新角度
void RotateCommand::execute()
{
    if (m_shape) {
        m_shape->setRotationAngle(m_newAngle);
        if (m_view) {
            m_view->update();
        }
    }
}

// 撤销命令 = 将图形恢复为旧角度
void RotateCommand::undo()
{
    if (m_shape) {
        m_shape->setRotationAngle(m_oldAngle);
        if (m_view) {
            m_view->update();
        }
    }
}
