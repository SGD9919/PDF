#include "pdfviewerpage.h"
#include <QPainter>
#include <QWidget>
#include <QStyleOptionGraphicsItem>

PDFViewerPage::PDFViewerPage(QGraphicsItem *parent)
    : QGraphicsPixmapItem{parent}
{

}

PDFViewerPage::PDFViewerPage(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsPixmapItem{pixmap,parent}
{

}

PDFViewerPage::~PDFViewerPage()
{

}

QPixmap PDFViewerPage::pixmap() const
{
    return m_pixMap;
}

void PDFViewerPage::setPixmap(const QPixmap &pixmap)
{
    m_pixMap=pixmap;
    //for parent init
    QGraphicsPixmapItem::setPixmap(pixmap);
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    m_expireRect=QRectF(offset(), QSizeF(m_pixMap.size()) / m_pixMap.devicePixelRatio());
#else
    m_expireRect=QRectF(offset(), QSizeF(m_pixMap.size()));
#endif
    updateDrawRect();
}

void PDFViewerPage::setOffset(const QPointF &offset)
{
    QGraphicsPixmapItem::setOffset(offset);
    updateDrawRect();
}

void PDFViewerPage::setOffset(qreal ox, qreal oy)
{
    QGraphicsPixmapItem::setOffset(ox,oy);
    updateDrawRect();
}

void PDFViewerPage::updatePixmap(const QPixmap &pixmap)
{
    m_pixMap=pixmap;
    updateDrawRect();
    update();
}

void PDFViewerPage::updateDrawRect()
{
    QRectF target=m_expireRect;
    QRectF source=QRectF(m_pixMap.rect());
    //use min scale to display image
    qreal scaleW=target.width()/source.width();
    qreal scaleH=target.height()/source.height();
    qreal useScale=qMin(scaleW,scaleH);
    m_drawRect=QRectF(offset().x(),offset().y(),source.width()*useScale,source.height()*useScale);
}

void PDFViewerPage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::SmoothPixmapTransform,
                           (transformationMode() == Qt::SmoothTransformation));

    painter->drawPixmap(m_drawRect,m_pixMap,QRectF(m_pixMap.rect()));

    if (option->state & QStyle::State_Selected)
        qt_graphicsItem_highlightSelected(this, painter, option);
}

QRectF PDFViewerPage::boundingRect() const
{
    return m_expireRect;
}
