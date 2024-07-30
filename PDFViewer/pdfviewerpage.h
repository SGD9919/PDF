#ifndef PDFVIEWERPAGE_H
#define PDFVIEWERPAGE_H

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>

class PDFViewerPage : public QGraphicsPixmapItem
{
public:
    explicit PDFViewerPage(QGraphicsItem *parent = nullptr);
    explicit PDFViewerPage(const QPixmap &pixmap, QGraphicsItem *parent = nullptr);
    virtual~PDFViewerPage();


    QPixmap pixmap() const;
    void setPixmap(const QPixmap& pixmap);
    void setOffset(const QPointF& offset);
    void setOffset(qreal ox,qreal oy);
    //用于更新图片显示
    void updatePixmap(const QPixmap &pixmap);
private:
    void updateDrawRect();
private:
    QPixmap m_pixMap;
    QRectF m_expireRect;
    QRectF m_drawRect;

//qt 5.15.2 src code
private:
    static void qt_graphicsItem_highlightSelected(
            QGraphicsItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option)
    {
        const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
        if (qFuzzyIsNull(qMax(murect.width(), murect.height())))
            return;

        const QRectF mbrect = painter->transform().mapRect(item->boundingRect());
        if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
            return;

        qreal itemPenWidth;
        switch (item->type()) {
        case QGraphicsEllipseItem::Type:
            itemPenWidth = static_cast<QGraphicsEllipseItem *>(item)->pen().widthF();
            break;
        case QGraphicsPathItem::Type:
            itemPenWidth = static_cast<QGraphicsPathItem *>(item)->pen().widthF();
            break;
        case QGraphicsPolygonItem::Type:
            itemPenWidth = static_cast<QGraphicsPolygonItem *>(item)->pen().widthF();
            break;
        case QGraphicsRectItem::Type:
            itemPenWidth = static_cast<QGraphicsRectItem *>(item)->pen().widthF();
            break;
        case QGraphicsSimpleTextItem::Type:
            itemPenWidth = static_cast<QGraphicsSimpleTextItem *>(item)->pen().widthF();
            break;
        case QGraphicsLineItem::Type:
            itemPenWidth = static_cast<QGraphicsLineItem *>(item)->pen().widthF();
            break;
        default:
            itemPenWidth = 1.0;
        }
        const qreal pad = itemPenWidth / 2;

        const qreal penWidth = 0; // cosmetic pen

        const QColor fgcolor = option->palette.windowText().color();
        const QColor bgcolor( // ensure good contrast against fgcolor
                              fgcolor.red()   > 127 ? 0 : 255,
                              fgcolor.green() > 127 ? 0 : 255,
                              fgcolor.blue()  > 127 ? 0 : 255);

        painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));

        painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
    }

    // QGraphicsItem interface
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // QGraphicsItem interface
public:
    QRectF boundingRect() const override;
};

#endif // PDFVIEWERPAGE_H
