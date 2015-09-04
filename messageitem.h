#ifndef MESSAGEITEM_H
#define MESSAGEITEM_H

#include <QGraphicsTextItem>


class MessageItem : public QGraphicsTextItem
{
public:
    MessageItem();

public:
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);

};

#endif // MESSAGEITEM_H
