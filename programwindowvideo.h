#ifndef PROGRAMWINDOWVIDEO_H
#define PROGRAMWINDOWVIDEO_H

#include "programwindow.h"

class ProgramWindowVideo : public ProgramWindow
{
    Q_OBJECT
public:
    explicit ProgramWindowVideo(QStringList fileNames, QWidget *parent);
private:
    void initReport(bool writeCoords = false);
    void writeReport(std::vector<MagellanHit> hits);
    bool writeCoords;
    int hitsNum;

    void readCurrentFrame();
    void preloadFrames(int frameStart, int frameSize);
    std::vector<MagellanHit> getMagellanHits(BannerInfo info, std::vector<MatchInfo> matches, int frameNum);

private slots:
    void on_buildReportBtn_clicked();
private:
    cv::VideoCapture capture;
    QString videoFileName;
};

#endif // PROGRAMWINDOWVIDEO_H
