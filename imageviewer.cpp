#include <QtGui>

#include "imageviewer.h"
#include "qfiledialog.h"
#include "qmessagebox.h"

 ImageViewer::ImageViewer(QWidget *parent)
 {
     imageLabel = new MouseLabel();
     imageLabel->setBackgroundRole(QPalette::Base);
     imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
     imageLabel->setScaledContents(true);

     scrollArea = new QScrollArea;
     scrollArea->setBackgroundRole(QPalette::Dark);
     scrollArea->setWidget(imageLabel);
     setCentralWidget(scrollArea);

     createActions();
     createMenus();

     scaleFactor = 1.0;
 }

 void ImageViewer::setImage(QImage image, double initScale)
 {
    imageLabel->setPixmap(QPixmap::fromImage(image));
    if (initScale != 0.0)
        scaleFactor = initScale;

    scaleImage(1.0);

    zoomInAct->setEnabled(scaleFactor < 10.0);
    //zoomOutAct->setEnabled(scaleFactor > 1.0);
    zoomOutAct->setEnabled(scaleFactor > 0.1);
 }

 void ImageViewer::zoomIn()
 {
     scaleImage(1.25);
 }

 void ImageViewer::zoomOut()
 {
     scaleImage(0.8);
 }

 void ImageViewer::normalSize()
 {
     imageLabel->adjustSize();
     scaleFactor = 1.0;
 }

 void ImageViewer::createActions()
 { 
     zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
     zoomInAct->setShortcut(tr("Ctrl++"));
     zoomInAct->setEnabled(false);
     connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

     zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
     zoomOutAct->setShortcut(tr("Ctrl+-"));
     zoomOutAct->setEnabled(false);
     connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

     normalSizeAct = new QAction(tr("&Normal Size"), this);
     normalSizeAct->setShortcut(tr("Ctrl+S"));
     normalSizeAct->setEnabled(false);
     connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

     aboutAct = new QAction(tr("&Hints"), this);
     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
 }

 void ImageViewer::createMenus()
 {
     viewMenu = new QMenu(tr("&View"), this);
     viewMenu->addAction(zoomInAct);
     viewMenu->addAction(zoomOutAct);
     viewMenu->addAction(normalSizeAct);

     helpMenu = new QMenu(tr("&Help"), this);
     helpMenu->addAction(aboutAct);

     menuBar()->addMenu(viewMenu);
     menuBar()->addMenu(helpMenu);
 }

 void ImageViewer::scaleImage(double factor)
 {
     Q_ASSERT(imageLabel->pixmap());
     scaleFactor *= factor;
     imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

     adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
     adjustScrollBar(scrollArea->verticalScrollBar(), factor);

     zoomInAct->setEnabled(scaleFactor < 10.0);
     //zoomOutAct->setEnabled(scaleFactor > 1.0);
     zoomOutAct->setEnabled(scaleFactor > 0.1);
 }

 void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
 {
     scrollBar->setValue(int(factor * scrollBar->value()
                             + ((factor - 1) * scrollBar->pageStep()/2)));
 }

 void ImageViewer::about()
 {
     QMessageBox::about(this, "Help", "Left mouse - start edit mode for banner selection.");
 }


