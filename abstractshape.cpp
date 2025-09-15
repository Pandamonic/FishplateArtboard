// ----------------- abstractshape.cpp (请完整替换此文件内容) -----------------

#include "abstractshape.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QPointF>
#include <QRectF>
#include <QDebug>

// 必须包含所有具体的图形类头文件，因为我们要在这里创建它们
#include "lineshape.h"
#include "rectangleshape.h"
#include "ellipseshape.h"
#include "starshape.h"
#include "freehandpathshape.h"
#include "eraserpathshape.h"
#include "groupshape.h"

// 工厂方法的完整实现
AbstractShape* AbstractShape::fromJsonObject(const QJsonObject &json)
{
    // 1. 读取所有图形共有的属性
    QString type = json["type"].toString();
    int penWidth = json["pen_width"].toInt();
    QColor borderColor(json["border_color"].toString());
    bool isFilled = json["is_filled"].toBool(false);
    QColor fillColor(json["fill_color"].toString());
    // 读取旋转角度，如果JSON中不存在此字段，则默认为0.0
    qreal rotation = json["rotation"].toDouble(0.0);

    // 2. 读取几何数据
    QJsonObject geometry = json["geometry"].toObject();

    // 3. 声明一个空的图形指针，用于接收新创建的图形
    AbstractShape *shape = nullptr;

    // 4. 根据类型创建不同的图形对象
    if (type == "Rectangle") {
        QRectF rect(geometry["x"].toDouble(), geometry["y"].toDouble(), geometry["width"].toDouble(), geometry["height"].toDouble());
        shape = new RectangleShape(rect, borderColor, penWidth, isFilled, fillColor);
    }
    else if (type == "Ellipse") {
        QRectF rect(geometry["x"].toDouble(), geometry["y"].toDouble(), geometry["width"].toDouble(), geometry["height"].toDouble());
        shape = new EllipseShape(rect, borderColor, penWidth, isFilled, fillColor);
    }
    else if (type == "Star") {
        QRectF rect(geometry["x"].toDouble(), geometry["y"].toDouble(), geometry["width"].toDouble(), geometry["height"].toDouble());
        int numPoints = geometry["num_points"].toInt(5); // 读取角点数，如果不存在则默认为5
        shape = new StarShape(rect, borderColor, penWidth, isFilled, fillColor, numPoints);
    }
    else if (type == "Line") {
        QJsonArray p1Array = geometry["p1"].toArray();
        QJsonArray p2Array = geometry["p2"].toArray();
        QPoint p1(p1Array[0].toInt(), p1Array[1].toInt());
        QPoint p2(p2Array[0].toInt(), p2Array[1].toInt());
        shape = new LineShape(p1, p2, borderColor, penWidth);
    }
    else if (type == "Freehand") {
        QVector<QPoint> points;
        QJsonArray pointsArray = geometry["points"].toArray();
        for (const QJsonValue &val : pointsArray) {
            QJsonArray pointArray = val.toArray();
            points.append(QPoint(pointArray[0].toInt(), pointArray[1].toInt()));
        }
        shape = new FreehandPathShape(points, borderColor, penWidth);
    }
    else if (type == "NormalEraser") {
        QVector<QPoint> points;
        QJsonArray pointsArray = geometry["points"].toArray();
        for (const QJsonValue &val : pointsArray) {
            QJsonArray pointArray = val.toArray();
            points.append(QPoint(pointArray[0].toInt(), pointArray[1].toInt()));
        }
        // 橡皮擦的颜色通常是固定的背景色，但我们也从文件加载以保持数据一致性
        shape = new EraserPathShape(points, penWidth, borderColor);
    }
    else if (type == "Group") {
        QJsonArray childrenArray = json["children"].toArray();
        QList<AbstractShape*> children;
        for (const QJsonValue& childVal : childrenArray) {
            // 递归调用 fromJsonObject 来创建每一个子图形
            AbstractShape* childShape = fromJsonObject(childVal.toObject());
            if (childShape) {
                children.append(childShape);
            }
        }
        if (!children.isEmpty()) {
            shape = new GroupShape(children);
        }
    }

    else {
        qWarning() << "Unknown shape type in JSON:" << type;
    }

    // 5. 如果图形被成功创建，就为它设置旋转角度
    if (shape) {
        shape->setRotationAngle(rotation);
    }

    return shape;
}
