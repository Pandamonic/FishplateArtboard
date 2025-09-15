#include "movemultipleshapescommand.h"
#include "abstractshape.h"
#include "artboardview.h"

MoveMultipleShapesCommand::MoveMultipleShapesCommand(const QList<AbstractShape*> &shapes, const QPoint &offset, ArtboardView *view)
    : m_shapes(shapes), m_offset(offset), m_view(view)
{
}

MoveMultipleShapesCommand::~MoveMultipleShapesCommand()
{
}

void MoveMultipleShapesCommand::execute()
{
    for (AbstractShape* shape : m_shapes) {
        shape->moveBy(m_offset);
    }
    if (m_view) {
        m_view->update();
    }
}

void MoveMultipleShapesCommand::undo()
{
    // 以相反的偏移量移回
    for (AbstractShape* shape : m_shapes) {
        shape->moveBy(-m_offset);
    }
    if (m_view) {
        m_view->update();
    }
}
