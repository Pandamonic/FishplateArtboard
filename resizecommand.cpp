#include "resizecommand.h"
#include "abstractshape.h"
#include "artboardview.h"

ResizeCommand::ResizeCommand(AbstractShape *shape, ArtboardView *view, const QRect &oldRect, const QRect &newRect)
    : m_shape(shape), m_view(view), m_oldRect(oldRect), m_newRect(newRect)
{
}

void ResizeCommand::execute()
{
    if (m_shape) {
        m_shape->setGeometry(m_newRect);
        if (m_view) {
            // 命令执行后，要求视图重绘以显示最新状态
            m_view->update();
        }
    }
}

void ResizeCommand::undo()
{
    if (m_shape) {
        m_shape->setGeometry(m_oldRect);
        if (m_view) {
            // 撤销后，同样要求视图重绘
            m_view->update();
        }
    }
}
