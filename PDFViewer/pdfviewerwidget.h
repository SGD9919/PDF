#ifndef PDFVIEWERWIDGET_H
#define PDFVIEWERWIDGET_H

#include <QObject>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsLinearLayout>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QFuture>
#include <QMap>
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QtConcurrent>
#else
#include <QtCore>
#endif
#include <QComboBox>

#include "qpdfium.h"
#include "pdfviewerpage.h"

class PDFViewerWidget : public QWidget
{   
    Q_OBJECT
public:
    explicit PDFViewerWidget(QWidget *parent = nullptr);
    ~PDFViewerWidget();
    QList<int> getCurrentVisiblePages();

public:
    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void clearPdfDisplayData();
    void loadPdfFromFile(const QString& file);

private slots:
    //operation
    void showLastPage();
    void showNextPage();
    void showSelfDefinedPage();
    void showSelfDefinedPage(int index);
    void showPage(int page);

private slots:
    void reloadImageTimeOut();
    void startImageLoader(const QList<int>& loadOrder);
    void waitLastLoadFinish();

    // QWidget interface
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QMap<int,PDFViewerPage*> m_imageItemMap;//page and item
    QMap<PDFViewerPage*,int> m_imageItemPageMap;//item and page
    QGraphicsView* m_imageView;
    QGraphicsScene* m_imageScene;
    QPdfium* m_pdf;
    bool m_scaleEnable;
    QTimer* m_reloadTimer;
    qreal m_currentScale;
    //多线程读取pdf并加载
    QList<QFuture<int>> m_pdfLoaders;
    bool m_requestLoaderStop;//请求停止加载

    //操作相关
     QComboBox* m_pageComboBox;
    int m_currentVisiblePage;
};

#endif // PDFVIEWERWIDGET_H
