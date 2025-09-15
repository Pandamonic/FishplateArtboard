#include "addmultipleshapescommand.h"
#include "artboardview.h"
#include "abstractshape.h"

AddMultipleShapesCommand::AddMultipleShapesCommand(const QList<AbstractShape*> &shapes, ArtboardView *view)
    : m_shapesToAdd(shapes), m_view(view), m_isOwnedByView(false)
{
}

AddMultipleShapesCommand::~AddMultipleShapesCommand()
{
    if (!m_isOwnedByView) {
        qDeleteAll(m_shapesToAdd);
    }
}

void AddMultipleShapesCommand::execute()
{
    if (m_view) {
        m_view->shapesList.append(m_shapesToAdd);
        m_isOwnedByView = true;
        m_view->update();
    }
}

void AddMultipleShapesCommand::undo()
{
    if (m_view) {
        for(AbstractShape* shape : m_shapesToAdd) {
            m_view->shapesList.removeOne(shape);
        }
        m_isOwnedByView = false;
        m_view->update();
    }
}
