#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>

#include "qaction.h"
#include "qlabel.h"
#include "qmenu.h"
#include "qscrollarea.h"
#include "qscrollbar.h"
#include "qmenubar.h"
#include "qmessagebox.h"
#include "mouselabel.h"

 class ImageViewer : public QMainWindow
 {
     Q_OBJECT

 public:
     explicit ImageViewer(QWidget *parent = 0);
     void setImage(QImage image, double initScale = 0.0);
     int getPosX(){ return lround(imageLabel->x / scaleFactor); }
     int getPosY(){ return lround(imageLabel->y / scaleFactor); }
     MouseLabel *imageLabel;

 private slots:
     void about();
     void zoomIn();
     void zoomOut();
     void normalSize();

 private:
     void createActions();
     void createMenus();
     void scaleImage(double factor);
     void adjustScrollBar(QScrollBar *scrollBar, double factor);

     QScrollArea *scrollArea;
     double scaleFactor;

     QAction *zoomInAct;
     QAction *zoomOutAct;
     QAction *normalSizeAct;
     QAction *aboutAct;

     QMenu *viewMenu;
     QMenu *helpMenu;
 };

 #endif
