#ifndef PROGRAMWINDOW_H
#define PROGRAMWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QFile>
#include <QTextStream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include "objectdetector.h"
#include <QTime>
#include "imageviewer.h"
#include "structures.h"

#include "ui_programwindow.h"
namespace Ui {
class ProgramWindow;
}

class ProgramWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ProgramWindow(QWidget *parent = 0);
    ~ProgramWindow();
    
protected slots:

    virtual void on_buildReportBtn_clicked();

    void on_minuteSb_sliderReleased();

    void on_gotoFrameSb_valueChanged(int arg1);

    void on_nextButton_clicked();

    void on_prevButton_clicked();

    void on_framesListWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_paramsDialogBtn_clicked();

    void on_createBannerButton_clicked();

    void on_detectButton_clicked();

    void addPoint();

    void on_saveButton_clicked();

    void on_loadButton_clicked();

    void on_deleteButton_clicked();

    void on_checkAllBox_toggled(bool checked);

    void on_bannersListWidget_itemDoubleClicked(QListWidgetItem *item);

protected:
    Ui::ProgramWindow *ui;
    QTextStream reportStream;
    cv::Mat curr_frame;
    int frame_count;
    int curr_frame_num;
    int curr_min;
    QTime timer;
    double initScale;

    bool in_process;
    bool break_process;
    ObjectDetector *objectDetector;
    std::vector<BannerInfo> banners;
    Parameters *params;

    virtual void readCurrentFrame() {}
    void drawFrame(cv::Mat frame, bool reset);
    virtual void preloadFrames(int frameStart, int frameSize);
    void updateSliders();

private:
    QString workDir;
    std::vector<cv::Point> points;
    cv::Mat edit_frame;

    void addBanner(BannerInfo banner);
};

#endif // PROGRAMWINDOW_H
