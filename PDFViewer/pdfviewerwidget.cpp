#include "pdfviewerwidget.h"
#include <QDebug>
#include <QGridLayout>
#include <QScrollBar>
#include <QRegion>

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QLineEdit>

PDFViewerWidget::PDFViewerWidget(QWidget *parent)
    : QWidget{parent},m_pdf(NULL),m_scaleEnable(false),m_currentVisiblePage(-1)
{
    //初始化
    QVBoxLayout* vboxLayout=new QVBoxLayout();

    //pdf loader
    m_pdf=new QPdfium;
    //scene
    m_imageScene=new QGraphicsScene;
    //view
    m_imageView=new QGraphicsView(m_imageScene);
    m_imageView->setRenderHints(QPainter::SmoothPixmapTransform);
    m_imageView->setDragMode(QGraphicsView::ScrollHandDrag); // 允许拖动
    //    m_imageView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    //    m_imageView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    //    m_imageView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_imageView->setBackgroundBrush(QColor(170, 170, 170));
    m_imageView->setInteractive(true);
    m_imageView->setFocusPolicy(Qt::WheelFocus);
    m_imageView->installEventFilter(this);
    vboxLayout->addWidget(m_imageView,1);

    //操作栏
    QHBoxLayout* operationLayout=new QHBoxLayout();

    QSpacerItem* spacerStart=new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
    operationLayout->addSpacerItem(spacerStart);

    QPushButton* lastPageBtn=new QPushButton;
    lastPageBtn->setText("<<");
    lastPageBtn->setMinimumWidth(80);
    lastPageBtn->setFixedHeight(35);
    lastPageBtn->setFocusPolicy(Qt::NoFocus);
    connect(lastPageBtn,SIGNAL(pressed()),this,SLOT(showLastPage()));
    operationLayout->addWidget(lastPageBtn);

    m_pageComboBox=new QComboBox;
    m_pageComboBox->setEditable(true);
    m_pageComboBox->setMinimumWidth(100);
    m_pageComboBox->setFixedHeight(35);
    connect(m_pageComboBox->lineEdit(),SIGNAL(returnPressed()),this,SLOT(showSelfDefinedPage()));
    connect(m_pageComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(showSelfDefinedPage(int)));

    operationLayout->addWidget(m_pageComboBox);

    QPushButton* nextPageBtn=new QPushButton;
    nextPageBtn->setText(">>");
    nextPageBtn->setMinimumWidth(100);
    nextPageBtn->setFixedHeight(35);
    nextPageBtn->setFocusPolicy(Qt::NoFocus);
    connect(nextPageBtn,SIGNAL(pressed()),this,SLOT(showNextPage()));
    operationLayout->addWidget(nextPageBtn);

    QSpacerItem* spacerEnd=new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
    operationLayout->addSpacerItem(spacerEnd);

    vboxLayout->addLayout(operationLayout,0);


    this->setLayout(vboxLayout);

    m_reloadTimer=new QTimer;
    m_reloadTimer->setInterval(500);
    m_reloadTimer->setSingleShot(true);
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    connect(m_reloadTimer,&QTimer::timeout,this,&PDFViewerWidget::reloadImageTimeOut);
#else
    connect(m_reloadTimer,SIGNAL(timeout()),this,SLOT(reloadImageTimeOut()));
#endif
}

PDFViewerWidget::~PDFViewerWidget()
{
    if(m_reloadTimer)
    {
        m_reloadTimer->stop();
        delete m_reloadTimer;
        m_reloadTimer=NULL;
    }
    //等待多线程加载完成
    waitLastLoadFinish();
    if(m_pdf)
    {
        delete m_pdf;
        m_pdf=NULL;
    }
}

QList<int> PDFViewerWidget::getCurrentVisiblePages()
{
    QList<QGraphicsItem*> visbleItems= m_imageView->items(m_imageView->viewport()->rect());
    QList<int> visibleItemPages;
    for(int i=0;i<visbleItems.size();i++)
    {
        PDFViewerPage* page=(PDFViewerPage*)visbleItems[i];
        visibleItemPages.append(m_imageItemPageMap[page]);
    }
    qSort(visibleItemPages.begin(),visibleItemPages.end());
    return visibleItemPages;
}

bool PDFViewerWidget::eventFilter(QObject *object, QEvent *event)
{
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent* keyEvent=(QKeyEvent*)event;
        if(keyEvent->key()==Qt::Key_Control)
        {
            m_scaleEnable=true;
            m_imageView->verticalScrollBar()->setEnabled(false);
            m_imageView->horizontalScrollBar()->setEnabled(false);
        }
    }
    else if(event->type()==QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent=(QKeyEvent*)event;
        if(keyEvent->key()==Qt::Key_Control)
        {
            m_scaleEnable=false;
            m_imageView->verticalScrollBar()->setEnabled(true);
            m_imageView->horizontalScrollBar()->setEnabled(true);
        }
    }
    else if(event->type()==QEvent::Wheel)
    {
        if(m_scaleEnable)
        {
            QWheelEvent* whEvent=(QWheelEvent*)event;
            if(whEvent->delta()>0)
            {
                if(m_currentScale<=5)
                {
                    m_imageView->scale(1.1,1.1);
                    m_currentScale*=1.1;
                }
            }
            else
            {
                if(m_currentScale>=0.05)
                {
                    m_imageView->scale(0.9,0.9);
                    m_currentScale*=0.9;
                }
            }
            event->accept();
            m_reloadTimer->start();
            return true;
        }
    }
    return QWidget::eventFilter(object,event);
}

void PDFViewerWidget::clearPdfDisplayData()
{
    if(!m_pdf)
        return;
    //等待多线程加载完成
    waitLastLoadFinish();
    //reset
    m_imageScene->clear();
    m_imageView->resetTransform();
    m_imageItemMap.clear();
    m_imageItemPageMap.clear();
    m_pageComboBox->clear();
}

void PDFViewerWidget::loadPdfFromFile(const QString &file)
{
    if(!m_pdf)
        return;
    //等待多线程加载完成
    waitLastLoadFinish();
    //reset
    m_imageScene->clear();
    m_imageView->resetTransform();
    m_imageItemMap.clear();
    m_imageItemPageMap.clear();
    //load
    m_pdf->loadFile(file);
    if(m_pdf->isValid())
    {
        int pdfCount=m_pdf->pageCount();
        //读取所有页面，然后渲染
        qreal startOffSetX=0;
        qreal startOffSetY=0;

        for(int i=0;i<pdfCount;i++)
        {
            PDFViewerPage* imageItem=new PDFViewerPage();
            QPdfiumPage pdfPage= m_pdf->page(i);
            QImage image= pdfPage.image();
            QPixmap pixmap=QPixmap::fromImage(image);
            imageItem->setOffset(startOffSetX,startOffSetY);
            imageItem->setPixmap(pixmap);
            startOffSetY+=pixmap.height()+10;
            m_imageScene->addItem(imageItem);
            m_imageItemMap.insert(i,imageItem);
            m_imageItemPageMap.insert(imageItem,i);
        }
        m_currentScale=1.0;
        if(pdfCount>0)
            showPage(0);
    }
    //更新ui选择
    m_pageComboBox->clear();
    QList<int> pages=m_imageItemMap.keys();
    for(int i=0;i<pages.size();i++)
    {
        m_pageComboBox->addItem(QString::number(pages[i]+1));
    }
}

void PDFViewerWidget::showLastPage()
{
    if(m_imageItemMap.size()<=0)
        return;
    //获取当前可见页
    QList<int> visiblePages= getCurrentVisiblePages();
    int lastPage=0;
    if(visiblePages.size()>0)
    {
        lastPage=visiblePages.first();
        if(lastPage-1<0)
        {
            lastPage=0;
        }
        else
        {
            lastPage-=1;
        }
    }

    showPage(lastPage);
}

void PDFViewerWidget::showNextPage()
{
    if(m_imageItemMap.size()<=0)
        return;
    //获取当前可见页
    QList<int> visiblePages= getCurrentVisiblePages();
    int nextPage=m_imageItemMap.lastKey();
    if(visiblePages.size()>0)
    {
        nextPage=visiblePages.last();
        if(nextPage+1>=m_imageItemMap.size())
        {
            nextPage=m_imageItemMap.size()-1;
        }
        else
        {
            nextPage+=1;
        }
    }

    showPage(nextPage);
}

void PDFViewerWidget::showSelfDefinedPage()
{
    //combox 自定义
    if(m_imageItemMap.size()<=0)
        return;
    int currentPage=m_pageComboBox->lineEdit()->text().toInt()-1;
    if(currentPage>=m_imageItemMap.size())
    {
        currentPage=m_imageItemMap.size()-1;
    }
    else if(currentPage<0)
    {
        currentPage=0;
    }
    showPage(currentPage);
}

void PDFViewerWidget::showSelfDefinedPage(int index)
{
    showSelfDefinedPage();
}

void PDFViewerWidget::showPage(int page)
{
    if(m_imageItemMap.contains(page))
    {
        m_currentVisiblePage=page;
        m_imageView->ensureVisible(m_imageItemMap[page],50,0);
        m_pageComboBox->setEditText(QString::number(page+1));
        m_imageView->setFocus();
    }
}

void PDFViewerWidget::reloadImageTimeOut()
{
    if(!m_pdf)
        return;
    if(m_imageItemMap.size()<=0)
        return;
    //for check only, pdfcount != itemCount must be some error happen
    int pdfCount=m_pdf->pageCount();
    int itemCount=m_imageItemMap.size();
    int reloadSize=qMin(pdfCount,itemCount);
    //获取当前可见的item
    //优先加载视图可见的页面，然后再多线程加载其他页面
    QList<int> visibleItemPages=getCurrentVisiblePages();
    QList<int> loadPageOrder;
    loadPageOrder.append(visibleItemPages);

    if(visibleItemPages.size()>0)
    {
        //双向加载顺序
        int lastStartPage=loadPageOrder.first()-1;
        int nextStartPage=loadPageOrder.last()+1;
        bool added=false;
        while (true) {
            added=false;
            if(lastStartPage>=0)
            {
                loadPageOrder.append(lastStartPage);
                lastStartPage-=1;
                added=true;
            }
            if(nextStartPage<reloadSize)
            {
                loadPageOrder.append(nextStartPage);
                nextStartPage+=1;
                added=true;
            }
            if(!added)
            {
                break;
            }
        }
    }
    else
    {
        //默认加载顺序
        for(int i=0;i<reloadSize;i++)
        {
            if(!visibleItemPages.contains(i))
            {
                loadPageOrder.append(i);
            }
            else
            {
                visibleItemPages.removeAll(i);
            }
        }
    }

    waitLastLoadFinish();
    startImageLoader(loadPageOrder);

}

void PDFViewerWidget::startImageLoader(const QList<int> &loadOrder)
{
    //根据加载的页面顺序进行处理
    m_requestLoaderStop=false;
    int threadCount=1;//目前不能使用多线程操作，只能单个线程处理
    //    int threadCount=loadOrder.size()/5;
    //    if(threadCount<=0)
    //    {
    //        threadCount=1;
    //    }
    for(int i=0;i<threadCount;i++)
    {
        int loadStart=i*5;
        int loadEnd=(i+1)*5;
        if(i+1>=threadCount)
        {
            loadEnd=loadOrder.size();
        }
        qreal currentScale=m_currentScale;
        //开始加载
        QFuture<int> loader=QtConcurrent::run(QThreadPool::globalInstance(),[&,loadOrder,loadStart,loadEnd,currentScale](){
            if(m_requestLoaderStop)
                return 0;
            for(int j=loadStart;j<loadEnd;j++)
            {
                if(m_requestLoaderStop)
                    return 0;
                if(!m_pdf)
                    return 0;
                int page=loadOrder[j];
                PDFViewerPage* imageItem=m_imageItemMap[page];
                QPdfiumPage pdfPage= m_pdf->page(page);
                QImage image= pdfPage.image(currentScale);
                QPixmap pixmap=QPixmap::fromImage(image);
                imageItem->updatePixmap(pixmap);
            }
            return 0;
        });
        m_pdfLoaders.append(loader);
    }
}

void PDFViewerWidget::waitLastLoadFinish()
{
    m_requestLoaderStop=true;
    for(int i=0;i<m_pdfLoaders.size();i++)
    {
        m_pdfLoaders[i].cancel();
        m_pdfLoaders[i].waitForFinished();
    }
    m_pdfLoaders.clear();
}

void PDFViewerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    //reset scale
    Q_UNUSED(event);
    m_imageView->resetTransform();
    m_currentScale=1.0;
    m_reloadTimer->start();
}
