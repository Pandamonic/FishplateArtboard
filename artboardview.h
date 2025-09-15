#ifndef ARTBOARDVIEW_H
#define ARTBOARDVIEW_H

#include <QWidget>
#include <QColor>
#include <QStack>
#include <QVector>
#include <QSet>
#include <QImage>

#include "shared_types.h"

class AbstractShape;
class AbstractCommand;

class ArtboardView : public QWidget
{
    Q_OBJECT

public:
    explicit ArtboardView(QWidget *parent = nullptr);
    ~ArtboardView() override;

    void setCurrentShape(ShapeType shape);
    void setCurrentDrawingColor(const QColor &color);
    void setCurrentPenWidth(int width);
    void setCurrentDrawingFillColor(const QColor &color);
    void enableFill(bool enable);
    void setBackgroundImage(const QImage &image);
    void clearBackgroundImage();
    void clearAllShapes();
    QImage renderToImage();
    int getCurrentPenWidth() const { return currentPenWidth; }
    QColor getCurrentDrawingColor() const { return currentDrawingColor; }
    QColor getCurrentDrawingFillColor() const { return currentDrawingFillColor; }
    void executeCommand(AbstractCommand *command);
    bool saveToDatabase(const QString &filePath);
    bool loadFromDatabase(const QString &filePath);
    const QList<AbstractShape*>& getSelectedShapes() const;

public slots:
    void undo();
    void redo();

signals:
    void undoAvailabilityChanged(bool available);
    void redoAvailabilityChanged(bool available);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    friend class AddShapeCommand;
    friend class DeleteShapeCommand;
    friend class ClearAllCommand;
    friend class MoveShapeCommand;
    friend class ResizeCommand; // 新增对 ResizeCommand 的友元
    friend class AddMultipleShapesCommand;
    friend class GroupCommand;     // <-- 添加这一行
    friend class UngroupCommand;   // <-- 添加这一行

private:
    // --- 绘图属性 ---
    QColor currentDrawingColor;
    int currentPenWidth;
    QColor currentDrawingFillColor;
    bool currentIsFilled;

    // --- 操作状态和数据 ---
    ShapeType currentShapeType;
    bool isCurrentlyDrawing;
    AbstractShape *currentShapeInProgressPtr;
    QPoint tempStartPoint;

    // --- 图形管理 ---
    QVector<AbstractShape *> shapesList;
    QList<AbstractShape*> m_selectedShapes;
    QPoint m_dragStartPoint_forCommand;

    // --- 命令栈 ---
    QStack<AbstractCommand *> undoStack;
    QStack<AbstractCommand *> redoStack;

    // --- 橡皮擦相关 ---
    QSet<AbstractShape*> shapesToDeleteInCurrentDrag;

    // --- 背景图 ---
    QImage m_backgroundImage;
    bool m_hasBackgroundImage;

    // --- 缩放/调整大小相关 ---
    QList<QRect> m_selectionHandles;
    bool m_isResizing;
    int m_currentHandleIndex;
    QRectF m_resizeOriginalRect;

    // --- 旋转相关状态 ---
    bool m_isRotating;
    QPointF m_rotationCenter;
    qreal m_rotationStartAngle; // <--- 就是这一行，确保它是存在的、没有被注释掉的

private: // 内部辅助函数
    void performStrokeEraseAtPoint(const QPoint &point);
    void clearCommandStacks();
    void clearRedoStack();
    void updateUndoRedoStatus();
    QPointF calculateRotationHandlePos() const;
};

#endif // ARTBOARDVIEW_H
