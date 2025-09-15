#ifndef ADDMULTIPLESHAPESCOMMAND_H
#define ADDMULTIPLESHAPESCOMMAND_H

#include "abstractcommand.h"
#include <QList>

class AbstractShape;
class ArtboardView;

class AddMultipleShapesCommand : public AbstractCommand
{
public:
    AddMultipleShapesCommand(const QList<AbstractShape*> &shapes, ArtboardView *view);
    ~AddMultipleShapesCommand() override;

    void execute() override;
    void undo() override;

private:
    QList<AbstractShape*> m_shapesToAdd;
    ArtboardView *m_view;
    bool m_isOwnedByView;
};

#endif // ADDMULTIPLESHAPESCOMMAND_H
