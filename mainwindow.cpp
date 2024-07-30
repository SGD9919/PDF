#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QGridLayout>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pdfView=new PDFViewerWidget;
    ui->centralwidget->layout()->addWidget(pdfView);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_action_PDF_triggered()
{
    QString file=QFileDialog::getOpenFileName(this,tr("选择要打开的PDF文件"),"","*.pdf");
    if(!file.isEmpty())
    {
        pdfView->loadPdfFromFile(file);
    }
}

