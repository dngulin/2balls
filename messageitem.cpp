#include "messageitem.h"

#include <QPainter>
#include <QFontDatabase>

MessageItem::MessageItem()
{
    QFont messageFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    messageFont.setPointSizeF(8);
    this->setFont(messageFont);
    this->setDefaultTextColor(Qt::yellow);
    this->setTextWidth(180);
}

void MessageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QPen messageOutline(Qt::yellow);
    messageOutline.setWidthF(1.5);

    painter->setBrush(Qt::black);
    painter->setPen(messageOutline);
    painter->drawRect(boundingRect());
    QGraphicsTextItem::paint(painter, o, w);
}
