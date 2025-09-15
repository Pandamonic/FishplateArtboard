#include "groupshape.h"
#include <QJsonArray>

// 构造函数接收一个子图形列表，并获得它们的所有权
GroupShape::GroupShape(const QList<AbstractShape*> &children)
    : AbstractShape(ShapeType::None), // 组本身没有类型
    m_children(children)
{
}

// 析构函数负责释放所有子图形的内存
GroupShape::~GroupShape()
{
    qDeleteAll(m_children);
}

// 绘制：依次调用所有子图形的绘制方法
void GroupShape::draw(QPainter *painter)
{
    for (AbstractShape* child : m_children) {
        child->draw(painter);
    }
}

// 获取包围盒：计算所有子图形包围盒的并集
QRect GroupShape::getBoundingRect() const
{
    if (m_children.isEmpty()) {
        return QRect();
    }

    QRect totalRect = m_children.first()->getBoundingRect();
    for (int i = 1; i < m_children.count(); ++i) {
        totalRect = totalRect.united(m_children.at(i)->getBoundingRect());
    }
    return totalRect;
}

// 点击判断：只要点中了任何一个子图形，就视为点中了组
bool GroupShape::containsPoint(const QPoint &point) const
{
    for (AbstractShape* child : m_children) {
        if (child->containsPoint(point)) {
            return true;
        }
    }
    return false;
}

// 移动：依次移动所有子图形
void GroupShape::moveBy(const QPoint &offset)
{
    for (AbstractShape* child : m_children) {
        child->moveBy(offset);
    }
}

// 序列化为JSON：保存组信息，并递归保存所有子图形
QJsonObject GroupShape::toJsonObject() const
{
    // [ 关键修正 ]
    // 之前我们错误地调用了基类中不存在的函数。
    // 正确的做法是直接创建一个新的、空的JSON对象。
    QJsonObject json;

    json["type"] = "Group";

    QJsonArray childrenArray;
    for (const AbstractShape* child : m_children) {
        childrenArray.append(child->toJsonObject());
    }
    json["children"] = childrenArray;

    return json;
}

// 获取中心点：整个组的包围盒的中心点
QPointF GroupShape::getCenter() const
{
    return getBoundingRect().center();
}

// 返回子图形列表的常量引用
const QList<AbstractShape *> &GroupShape::getChildren() const
{
    return m_children;
}

// 移交子图形列表的所有权（用于取消编组）
QList<AbstractShape *> GroupShape::takeChildren()
{
    QList<AbstractShape*> taken = m_children;
    m_children.clear(); // 清空列表，避免析构时重复删除
    return taken;
}

void GroupShape::addChildren(const QList<AbstractShape *> &children)
{
    m_children.append(children);
}

QRectF GroupShape::getCoreGeometry() const
{
    if (m_children.isEmpty()) {
        return QRectF();
    }

    // 正确的实现：计算所有子图形“核心几何体”的并集
    QRectF totalRect = m_children.first()->getCoreGeometry();
    for (int i = 1; i < m_children.count(); ++i) {
        totalRect = totalRect.united(m_children.at(i)->getCoreGeometry());
    }
    return totalRect;
}

void GroupShape::setRotationAngle(qreal newAngle)
{
    qreal oldAngle = getRotationAngle();
    qreal angleDelta = newAngle - oldAngle;

    // 1. 更新组自身的旋转角度
    AbstractShape::setRotationAngle(newAngle);

    // 2. 让所有子图形围绕“组的中心点”旋转 “angleDelta” 度
    QPointF groupCenter = getCenter();
    for (AbstractShape* child : m_children) {
        // 创建一个变换矩阵，它描述了“围绕组中心旋转”这个动作
        QTransform transform;
        transform.translate(groupCenter.x(), groupCenter.y());
        transform.rotate(angleDelta);
        transform.translate(-groupCenter.x(), -groupCenter.y());

        // 计算子图形原来的中心点，并应用上述变换得到新的中心点
        QPointF oldChildCenter = child->getCenter();
        QPointF newChildCenter = transform.map(oldChildCenter);

        // 通过移动子图形来更新其位置
        child->moveBy((newChildCenter - oldChildCenter).toPoint());

        // 更新子图形自身的旋转角度
        child->setRotationAngle(child->getRotationAngle() + angleDelta);
    }
}
