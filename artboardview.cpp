#include "artboardview.h"
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QImage>
#include <QPalette>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <stdexcept>

#include "lineshape.h"
#include "rectangleshape.h"
#include "freehandpathshape.h"
#include "ellipseshape.h"
#include "starshape.h"
#include "eraserpathshape.h"
#include "rotatecommand.h"
#include "abstractcommand.h"
#include "addshapecommand.h"
#include "deleteshapecommand.h"
#include "deletemultipleshapescommand.h"
#include "clearallcommand.h"
#include "moveshapecommand.h"
#include "resizecommand.h"
#include "movemultipleshapescommand.h"

ArtboardView::ArtboardView(QWidget *parent)
    : QWidget{parent},
    currentDrawingColor(Qt::black),
    currentPenWidth(2),
    currentDrawingFillColor(Qt::transparent),
    currentIsFilled(false),
    currentShapeType(ShapeType::None),
    isCurrentlyDrawing(false),
    currentShapeInProgressPtr(nullptr),
    m_dragStartPoint_forCommand(0,0),
    m_backgroundImage(),
    m_hasBackgroundImage(false),
    m_isResizing(false),
    m_currentHandleIndex(-1),
    m_isRotating(false)
{
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
    emit undoAvailabilityChanged(false);
    emit redoAvailabilityChanged(false);
}

ArtboardView::~ArtboardView()
{
    clearAllShapes();
}

void ArtboardView::clearAllShapes()
{
    qDeleteAll(shapesList);
    shapesList.clear();
    clearCommandStacks();

    m_selectedShapes.clear();

    if (currentShapeInProgressPtr) {
        delete currentShapeInProgressPtr;
        currentShapeInProgressPtr = nullptr;
    }
    isCurrentlyDrawing = false;
    update();
}

void ArtboardView::setCurrentShape(ShapeType shape)
{
    if (currentShapeType != shape) {
        currentShapeType = shape;
        if (currentShapeType != ShapeType::None) {
            if (!m_selectedShapes.isEmpty()) {
                m_selectedShapes.clear();
                update();
            }
        }
    }
}


void ArtboardView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 1. 绘制背景图 (逻辑不变)
    if (m_hasBackgroundImage && !m_backgroundImage.isNull()) {
        QRectF targetRect = this->rect();
        QImage scaledImage = m_backgroundImage.scaled(targetRect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qreal x = targetRect.left() + (targetRect.width() - scaledImage.width()) / 2.0;
        qreal y = targetRect.top() + (targetRect.height() - scaledImage.height()) / 2.0;
        painter.drawImage(QPointF(x, y), scaledImage);
    }

    // 2. 绘制所有已完成的图形 (逻辑不变)
    for (AbstractShape *shape : shapesList) {
        if (shape) {
            shape->draw(&painter);
        }
    }

    // 3. 绘制选中框和控制点 (这是我们修改的核心)
    m_selectionHandles.clear(); // 每一帧都先清空控制点列表

    if (!m_selectedShapes.isEmpty()) {
        // --- 如果选中了多个图形，为每个图形绘制一个普通的、非倾斜的蓝色虚线框 ---
        if (m_selectedShapes.count() > 1) {
            QPen selectionPen(Qt::blue, 1, Qt::DashLine);
            painter.setPen(selectionPen);
            painter.setBrush(Qt::NoBrush);
            for (AbstractShape* shape : m_selectedShapes) {
                painter.drawRect(shape->getBoundingRect().adjusted(-3, -3, 3, 3));
            }
        }
        // --- [ 关键修正 ] 如果只选中了一个图形，绘制倾斜的、精确的选择框和控制点 ---
        else if (m_selectedShapes.count() == 1) {
            AbstractShape* theOnlySelectedShape = m_selectedShapes.first();
            ShapeType type = theOnlySelectedShape->getType();

            // 1. 获取原始几何体并计算旋转后的顶点
            QRectF coreRect = theOnlySelectedShape->getCoreGeometry();
            QTransform transform;
            transform.translate(theOnlySelectedShape->getCenter().x(), theOnlySelectedShape->getCenter().y());
            transform.rotate(theOnlySelectedShape->getRotationAngle());
            transform.translate(-theOnlySelectedShape->getCenter().x(), -theOnlySelectedShape->getCenter().y());

            QPolygonF rotatedPolygon;
            rotatedPolygon << transform.map(coreRect.topLeft())
                           << transform.map(coreRect.topRight())
                           << transform.map(coreRect.bottomRight())
                           << transform.map(coreRect.bottomLeft());

            // 2. 绘制倾斜的蓝色虚线选择框
            QPen selectionPen(Qt::blue, 1, Qt::DashLine);
            painter.setPen(selectionPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawPolygon(rotatedPolygon);

            // 3. 在旋转后的顶点和边中点上绘制控制点
            if (type == ShapeType::Rectangle || type == ShapeType::Ellipse || type == ShapeType::Star) {
                int handleSize = 8;
                int halfHandleSize = handleSize / 2;

                // 计算旋转后的8个控制点位置
                QList<QPointF> handlePoints;
                handlePoints << rotatedPolygon[0] << rotatedPolygon[1] << rotatedPolygon[2] << rotatedPolygon[3]; // 4个角点
                handlePoints << (rotatedPolygon[0] + rotatedPolygon[1]) / 2.0; // 上边中点
                handlePoints << (rotatedPolygon[2] + rotatedPolygon[3]) / 2.0; // 下边中点
                handlePoints << (rotatedPolygon[3] + rotatedPolygon[0]) / 2.0; // 左边中点
                handlePoints << (rotatedPolygon[1] + rotatedPolygon[2]) / 2.0; // 右边中点

                painter.setPen(QPen(Qt::black, 1));
                painter.setBrush(Qt::white);
                for (const QPointF &pt : handlePoints) {
                    QRect handleRect(pt.x() - halfHandleSize, pt.y() - halfHandleSize, handleSize, handleSize);
                    m_selectionHandles.append(handleRect);
                    painter.drawRect(handleRect);
                }
            }

            // 4. 在旋转后的框顶部绘制旋转手柄
            if (type != ShapeType::NormalEraser) {
                QPointF topMidHandle = (rotatedPolygon[0] + rotatedPolygon[1]) / 2.0;

                // 计算旋转手柄的最终位置 (在顶部中点法线方向上偏移)
                QLineF topEdge(rotatedPolygon[0], rotatedPolygon[1]);
                QLineF normal = topEdge.normalVector();
                normal.setLength(20); // 偏移20像素
                QPointF rotationHandlePos = normal.p2() + (topMidHandle - normal.p1());

                painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
                painter.drawLine(topMidHandle, rotationHandlePos);

                painter.setBrush(Qt::green);
                painter.drawEllipse(rotationHandlePos, 5, 5);

                // 更新 calculateRotationHandlePos 的依赖
                // （注意：calculateRotationHandlePos 自身逻辑也需同步修改）
            }
        }
    }

    // 4. 绘制正在进行中的图形预览 (逻辑不变)
    if (isCurrentlyDrawing && currentShapeInProgressPtr && currentShapeType != ShapeType::None) {
        currentShapeInProgressPtr->draw(&painter);
    }
}

void ArtboardView::performStrokeEraseAtPoint(const QPoint &point)
{
    for (AbstractShape* shape : shapesList) {
        if (shape && shape->getType() != ShapeType::NormalEraser && shape->containsPoint(point)) {
            if (!shapesToDeleteInCurrentDrag.contains(shape)) {
                shapesToDeleteInCurrentDrag.insert(shape);
            }
        }
    }
}

void ArtboardView::mousePressEvent(QMouseEvent *event)
{
    // 仅当按下的是鼠标左键时才处理事件
    if (event->button() == Qt::LeftButton) {

        // ===================================================================
        // 分支一：当前为“选择”工具模式
        // ===================================================================
        if (currentShapeType == ShapeType::None) {
            // 每次点击都重置交互状态
            m_isRotating = false;
            m_isResizing = false;
            m_currentHandleIndex = -1;

            bool selectionHandled = false; // 用于标记事件是否已被控制点处理

            // 仅当只选中一个图形时，才检查是否点中了控制点
            if (m_selectedShapes.count() == 1) {
                AbstractShape* selectedShape = m_selectedShapes.first();

                // 1. 最优先检查：是否点中了旋转控制点？
                if (selectedShape->getType() != ShapeType::NormalEraser) {
                    QPointF rotationHandlePos = calculateRotationHandlePos();
                    QRectF handleArea(rotationHandlePos - QPointF(5, 5), QSizeF(10, 10));
                    if (handleArea.contains(event->pos())) {
                        m_isRotating = true;
                        isCurrentlyDrawing = true;
                        m_rotationCenter = selectedShape->getCenter();
                        m_rotationStartAngle = selectedShape->getRotationAngle();
                        m_dragStartPoint_forCommand = event->pos();
                        selectionHandled = true; // 标记事件已处理
                    }
                }

                // 2. 如果没有点中旋转点，再检查是否点中了缩放控制点
                if (!m_isRotating && !m_selectionHandles.isEmpty()) {
                    for (int i = 0; i < m_selectionHandles.size(); ++i) {
                        if (m_selectionHandles.at(i).contains(event->pos())) {
                            m_isResizing = true;
                            m_currentHandleIndex = i;
                            isCurrentlyDrawing = true;
                            // 关键：记录开始缩放时的原始“未旋转”几何体
                            m_resizeOriginalRect = selectedShape->getCoreGeometry();
                            selectionHandled = true; // 标记事件已处理
                            break;
                        }
                    }
                }
            }

            // 3. 如果没有操作控制点，才执行“选择/移动”逻辑
            if (!selectionHandled) {
                AbstractShape* shapeUnderMouse = nullptr;
                for (int i = shapesList.size() - 1; i >= 0; --i) {
                    AbstractShape* shape = shapesList.at(i);
                    if (shape && shape->getType() != ShapeType::NormalEraser && shape->containsPoint(event->pos())) {
                        shapeUnderMouse = shape;
                        break;
                    }
                }

                // 检查Shift键是否被按下
                bool isShiftPressed = (event->modifiers() & Qt::ShiftModifier);

                if (isShiftPressed) {
                    // --- Shift多选逻辑 ---
                    if (shapeUnderMouse) {
                        if (m_selectedShapes.contains(shapeUnderMouse)) {
                            m_selectedShapes.removeOne(shapeUnderMouse);
                        } else {
                            m_selectedShapes.append(shapeUnderMouse);
                        }
                    }
                } else {
                    // --- 原有的单选逻辑 ---
                    m_selectedShapes.clear();
                    if (shapeUnderMouse) {
                        m_selectedShapes.append(shapeUnderMouse);
                    }
                }

                update(); // 立即重绘，以显示新的选择状态

                // 如果选中了图形，则进入准备拖动的状态
                if (!m_selectedShapes.isEmpty()) {
                    isCurrentlyDrawing = true;
                    tempStartPoint = event->pos();
                    m_dragStartPoint_forCommand = event->pos();
                } else {
                    isCurrentlyDrawing = false;
                }
            }
        }
        // ===================================================================
        // 分支二至四：所有“绘图/橡皮擦工具”模式
        // ===================================================================
        else if (currentShapeType == ShapeType::StrokeEraser) {
            AbstractShape *shapeHit = nullptr;
            int hitIndex = -1;
            for (int i = shapesList.size() - 1; i >= 0; --i) {
                if (shapesList.at(i) && shapesList.at(i)->getType() != ShapeType::NormalEraser && shapesList.at(i)->containsPoint(event->pos())) {
                    shapeHit = shapesList.at(i);
                    hitIndex = i;
                    break;
                }
            }
            if (shapeHit) {
                this->executeCommand(new DeleteShapeCommand(shapeHit, this, hitIndex));
            }
        }
        else if (currentShapeType == ShapeType::DraggingStrokeEraser) {
            isCurrentlyDrawing = true;
            shapesToDeleteInCurrentDrag.clear();
            performStrokeEraseAtPoint(event->pos());
        }
        else {
            isCurrentlyDrawing = true;
            tempStartPoint = event->pos();
            if (currentShapeInProgressPtr) {
                delete currentShapeInProgressPtr;
                currentShapeInProgressPtr = nullptr;
            }
            switch (currentShapeType) {
            case ShapeType::Line:
                currentShapeInProgressPtr = new LineShape(tempStartPoint, tempStartPoint, currentDrawingColor, currentPenWidth);
                break;
            case ShapeType::Rectangle:
                currentShapeInProgressPtr = new RectangleShape(QRectF(tempStartPoint, tempStartPoint), currentDrawingColor, currentPenWidth, currentIsFilled, currentDrawingFillColor);
                break;
            case ShapeType::Freehand:
                currentShapeInProgressPtr = new FreehandPathShape(QVector<QPoint>() << tempStartPoint, currentDrawingColor, currentPenWidth);
                break;
            case ShapeType::NormalEraser:
                currentShapeInProgressPtr = new EraserPathShape(QVector<QPoint>() << tempStartPoint, currentPenWidth, palette().window().color());
                break;
            case ShapeType::Ellipse:
                currentShapeInProgressPtr = new EllipseShape(QRectF(tempStartPoint, tempStartPoint), currentDrawingColor, currentPenWidth, currentIsFilled, currentDrawingFillColor);
                break;
            case ShapeType::Star:
                currentShapeInProgressPtr = new StarShape(QRectF(tempStartPoint, tempStartPoint), currentDrawingColor, currentPenWidth, currentIsFilled, currentDrawingFillColor);
                break;
            default:
                isCurrentlyDrawing = false;
                currentShapeInProgressPtr = nullptr;
                break;
            }
        }
    } else {
        QWidget::mousePressEvent(event);
    }
}


void ArtboardView::mouseMoveEvent(QMouseEvent *event)
{
    // 确保是“左键按下并拖动”的状态
    if (!(event->buttons() & Qt::LeftButton) || !isCurrentlyDrawing) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if (currentShapeType == ShapeType::None) { // 选择工具模式
        if (m_selectedShapes.count() == 1) { // 仅当只选中一个图形时，才处理旋转和缩放
            AbstractShape* selectedShape = m_selectedShapes.first();

            if (m_isRotating) {
                // (旋转逻辑已正确，保持不变)
                QLineF startLine(m_rotationCenter, m_dragStartPoint_forCommand);
                QLineF currentLine(m_rotationCenter, QPointF(event->pos()));
                qreal angleDelta = startLine.angleTo(currentLine);
                qreal newAngle = m_rotationStartAngle - angleDelta;
                selectedShape->setRotationAngle(newAngle);
                update();
            }
            else if (m_isResizing) {
                // [ 最终的、最健壮的缩放逻辑 ]
                QTransform transform;
                transform.translate(selectedShape->getCenter().x(), selectedShape->getCenter().y());
                transform.rotate(selectedShape->getRotationAngle());
                transform.translate(-selectedShape->getCenter().x(), -selectedShape->getCenter().y());
                QTransform inverseTransform = transform.inverted();

                QPointF localCurrentMouse = inverseTransform.map(event->pos());
                QRectF newLocalRect;

                switch(m_currentHandleIndex) {
                // 角点控制：以对角为锚点
                case 0: newLocalRect.setTopLeft(localCurrentMouse); newLocalRect.setBottomRight(m_resizeOriginalRect.bottomRight()); break;
                case 1: newLocalRect.setTopRight(localCurrentMouse); newLocalRect.setBottomLeft(m_resizeOriginalRect.bottomLeft()); break;
                case 2: newLocalRect.setBottomRight(localCurrentMouse); newLocalRect.setTopLeft(m_resizeOriginalRect.topLeft()); break;
                case 3: newLocalRect.setBottomLeft(localCurrentMouse); newLocalRect.setTopRight(m_resizeOriginalRect.topRight()); break;

                    // 边框控制：只改变一个维度
                case 4: newLocalRect = m_resizeOriginalRect; newLocalRect.setTop(localCurrentMouse.y()); break;
                case 5: newLocalRect = m_resizeOriginalRect; newLocalRect.setBottom(localCurrentMouse.y()); break;
                case 6: newLocalRect = m_resizeOriginalRect; newLocalRect.setLeft(localCurrentMouse.x()); break;
                case 7: newLocalRect = m_resizeOriginalRect; newLocalRect.setRight(localCurrentMouse.x()); break;
                }
                // 使用 normalized() 来正确处理“翻转”的情况
                selectedShape->setGeometry(newLocalRect.normalized().toRect()); // <-- 在最后加上 .toRect()
                update();
            }
        }

        // 移动逻辑 (可作用于多选)
        if (!m_isRotating && !m_isResizing && !m_selectedShapes.isEmpty()) {
            QPoint offset = event->pos() - tempStartPoint;
            for (AbstractShape* shape : m_selectedShapes) {
                shape->moveBy(offset);
            }
            tempStartPoint = event->pos();
            update();
        }
    }
    else if (currentShapeType == ShapeType::DraggingStrokeEraser) {
        // 拖拽橡皮擦逻辑
        performStrokeEraseAtPoint(event->pos());
    }
    // 绘图逻辑
    else if (currentShapeInProgressPtr) {
        currentShapeInProgressPtr->updateShape(event->pos());
        update();
    }
}


// ----------------- artboardview.cpp (请将此函数完整地添加到文件中) -----------------
void ArtboardView::mouseReleaseEvent(QMouseEvent *event)
{
    // 确保是鼠标左键释放，并且之前确实处于一个交互操作中
    if (event->button() == Qt::LeftButton && isCurrentlyDrawing) {

        // 仅当只选中一个图形时，才处理旋转和缩放命令
        if (m_selectedShapes.count() == 1) {
            AbstractShape* selectedShape = m_selectedShapes.first();
            // 1. 如果刚刚完成的是一次旋转操作
            if (m_isRotating) {
                qreal finalAngle = selectedShape->getRotationAngle();
                if (qAbs(finalAngle - m_rotationStartAngle) > 0.01) {
                    selectedShape->setRotationAngle(m_rotationStartAngle);
                    executeCommand(new RotateCommand(selectedShape, this, m_rotationStartAngle, finalAngle));
                }
            }
            // 2. 如果完成的是一次缩放操作
            else if (m_isResizing) {
                QRect finalRect = selectedShape->getCoreGeometry().toRect();
                if (m_resizeOriginalRect.toRect() != finalRect) {
                    selectedShape->setGeometry(m_resizeOriginalRect.toRect());
                    executeCommand(new ResizeCommand(selectedShape, this, m_resizeOriginalRect.toRect(), finalRect));
                }
            }
        }

        // 3. 如果完成的是一次移动操作 (可作用于多选)
        if (currentShapeType == ShapeType::None && !m_isResizing && !m_isRotating && !m_selectedShapes.isEmpty()) {
            QPoint totalOffset = event->pos() - m_dragStartPoint_forCommand;
            if(!totalOffset.isNull()){
                // 先将所有图形移回原位
                for(AbstractShape* shape : m_selectedShapes) {
                    shape->moveBy(-totalOffset);
                }
                // 然后通过一个宏命令来执行移动，以便一次性撤销
                executeCommand(new MoveMultipleShapesCommand(m_selectedShapes, totalOffset, this));
            }
        }
        // 4. 如果完成的是拖拽橡皮擦
        else if (currentShapeType == ShapeType::DraggingStrokeEraser) {
            if (!shapesToDeleteInCurrentDrag.isEmpty()) {
                QList<DeleteShapeCommand*> commands;
                QList<AbstractShape*> sortedShapes = shapesToDeleteInCurrentDrag.values();
                std::sort(sortedShapes.begin(), sortedShapes.end(), [this](AbstractShape* a, AbstractShape* b){
                    return shapesList.indexOf(a) > shapesList.indexOf(b);
                });
                for (AbstractShape* shape : sortedShapes) {
                    int index = shapesList.indexOf(shape);
                    if (index != -1) {
                        commands.append(new DeleteShapeCommand(shape, this, index));
                    }
                }
                if(!commands.isEmpty()){
                    this->executeCommand(new DeleteMultipleShapesCommand(commands));
                }
                shapesToDeleteInCurrentDrag.clear();
            }
        }
        // 5. 如果完成的是一次绘图操作
        else if (currentShapeInProgressPtr) {
            currentShapeInProgressPtr->updateShape(event->pos());
            bool shapeIsValid = true;
            if (currentShapeInProgressPtr->getBoundingRect().width() < 2 || currentShapeInProgressPtr->getBoundingRect().height() < 2) {
                shapeIsValid = false;
            }

            if (shapeIsValid) {
                this->executeCommand(new AddShapeCommand(currentShapeInProgressPtr, this));
                currentShapeInProgressPtr = nullptr;
            } else {
                delete currentShapeInProgressPtr;
                currentShapeInProgressPtr = nullptr;
                update();
            }
        }

        // 统一重置所有交互状态
        isCurrentlyDrawing = false;
        m_isResizing = false;
        m_isRotating = false;
        m_currentHandleIndex = -1;

    } else {
        isCurrentlyDrawing = false;
        m_isResizing = false;
        m_isRotating = false;
        m_currentHandleIndex = -1;
        QWidget::mouseReleaseEvent(event);
    }
}

void ArtboardView::setCurrentDrawingColor(const QColor &color)
{
    if (this->currentDrawingColor != color) {
        this->currentDrawingColor = color;
    }
}

void ArtboardView::setCurrentPenWidth(int width)
{
    if (this->currentPenWidth != width && width > 0) {
        this->currentPenWidth = width;
    }
}

void ArtboardView::setCurrentDrawingFillColor(const QColor &color)
{
    if (currentDrawingFillColor != color) {
        currentDrawingFillColor = color;
        currentIsFilled = (color.alpha() != 0);
    }
}

void ArtboardView::enableFill(bool enable)
{
    if (currentIsFilled != enable) {
        currentIsFilled = enable;
    }
}

void ArtboardView::undo()
{
    if (!undoStack.isEmpty()) {
        AbstractCommand *commandToUndo = undoStack.pop();
        commandToUndo->undo();
        redoStack.push(commandToUndo);
        updateUndoRedoStatus();
    }
}

void ArtboardView::redo()
{
    if (!redoStack.isEmpty()) {
        AbstractCommand *commandToRedo = redoStack.pop();
        commandToRedo->execute();
        undoStack.push(commandToRedo);
        updateUndoRedoStatus();
    }
}

void ArtboardView::clearCommandStacks()
{
    qDeleteAll(undoStack);
    undoStack.clear();
    qDeleteAll(redoStack);
    redoStack.clear();
    updateUndoRedoStatus();
}

void ArtboardView::updateUndoRedoStatus()
{
    emit undoAvailabilityChanged(!undoStack.isEmpty());
    emit redoAvailabilityChanged(!redoStack.isEmpty());
}

void ArtboardView::clearRedoStack()
{
    if (!redoStack.isEmpty()) {
        qDeleteAll(redoStack);
        redoStack.clear();
    }
    updateUndoRedoStatus();
}

QImage ArtboardView::renderToImage()
{
    QImage imageToRender(this->size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&imageToRender);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(imageToRender.rect(), palette().window().color());
    if (m_hasBackgroundImage && !m_backgroundImage.isNull()) {
        QRectF targetRect = imageToRender.rect();
        QImage scaledImage = m_backgroundImage.scaled(targetRect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        qreal x = targetRect.left() + (targetRect.width() - scaledImage.width()) / 2.0;
        qreal y = targetRect.top() + (targetRect.height() - scaledImage.height()) / 2.0;
        painter.drawImage(QPointF(x, y), scaledImage);
    }
    for (AbstractShape *shape : shapesList) {
        if (shape) {
            shape->draw(&painter);
        }
    }
    return imageToRender;
}

void ArtboardView::executeCommand(AbstractCommand *command)
{
    if (!command) return;
    command->execute();
    undoStack.push(command);
    clearRedoStack();
}

void ArtboardView::setBackgroundImage(const QImage &image)
{
    if (image.isNull()) {
        if (m_hasBackgroundImage) {
            m_backgroundImage = QImage();
            m_hasBackgroundImage = false;
            update();
        }
    } else {
        m_backgroundImage = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        m_hasBackgroundImage = true;
        update();
    }
}

void ArtboardView::clearBackgroundImage()
{
    if (m_hasBackgroundImage) {
        m_backgroundImage = QImage();
        m_hasBackgroundImage = false;
        update();
    }
}


bool ArtboardView::saveToDatabase(const QString &filePath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "saver_connection");
    db.setDatabaseName(filePath);
    if (!db.open()) { qWarning() << "Error: Failed to connect to database." << db.lastError(); return false; }

    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS shapes (id INTEGER PRIMARY KEY, type TEXT, json_data TEXT)");
    db.transaction();
    try {
        query.exec("DELETE FROM shapes");
        for (AbstractShape *shape : shapesList) {
            if (!shape) continue;
            QJsonObject jsonObj = shape->toJsonObject();
            QString type = jsonObj["type"].toString();
            QString jsonString = QString(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
            query.prepare("INSERT INTO shapes (type, json_data) VALUES (:type, :json)");
            query.bindValue(":type", type);
            query.bindValue(":json", jsonString);
            if(!query.exec()){ throw std::runtime_error(query.lastError().text().toStdString()); }
        }
    } catch (const std::exception& e) {
        qWarning() << "Error during database transaction:" << e.what();
        db.rollback();
        db.close();
        QSqlDatabase::removeDatabase("saver_connection");
        return false;
    }
    db.commit();
    db.close();
    QSqlDatabase::removeDatabase("saver_connection");
    qDebug() << "Canvas saved successfully!";
    return true;
}

bool ArtboardView::loadFromDatabase(const QString &filePath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "loader_connection");
    db.setDatabaseName(filePath);
    if (!db.open()) { qWarning() << "Error: Failed to open database." << db.lastError(); return false; }

    clearAllShapes();
    QSqlQuery query(db);
    if(!query.exec("SELECT json_data FROM shapes")){ qWarning() << "Error: Failed to query shapes." << query.lastError(); db.close(); QSqlDatabase::removeDatabase("loader_connection"); return false; }

    while (query.next()) {
        QString jsonString = query.value(0).toString();
        QJsonObject jsonObj = QJsonDocument::fromJson(jsonString.toUtf8()).object();
        AbstractShape *shape = AbstractShape::fromJsonObject(jsonObj);
        if (shape) {
            shapesList.append(shape);
        }
    }
    db.close();
    QSqlDatabase::removeDatabase("loader_connection");
    update();
    qDebug() << "Canvas loaded successfully!";
    return true;
}


QPointF ArtboardView::calculateRotationHandlePos() const
{
    if (m_selectedShapes.count() != 1) {
        return QPointF();
    }
    AbstractShape* selectedShape = m_selectedShapes.first();

    // 复用和paintEvent中完全相同的逻辑来计算位置
    QRectF coreRect = selectedShape->getCoreGeometry();
    QTransform transform;
    transform.translate(selectedShape->getCenter().x(), selectedShape->getCenter().y());
    transform.rotate(selectedShape->getRotationAngle());
    transform.translate(-selectedShape->getCenter().x(), -selectedShape->getCenter().y());

    QPointF topLeft = transform.map(coreRect.topLeft());
    QPointF topRight = transform.map(coreRect.topRight());
    QPointF topMidHandle = (topLeft + topRight) / 2.0;

    QLineF topEdge(topLeft, topRight);
    QLineF normal = topEdge.normalVector();
    normal.setLength(20);
    QPointF rotationHandlePos = normal.p2() + (topMidHandle - normal.p1());

    return rotationHandlePos;
}

const QList<AbstractShape*>& ArtboardView::getSelectedShapes() const
{
    return m_selectedShapes;
}
