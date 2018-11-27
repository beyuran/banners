#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QStringList>
#include <opencv2/core.hpp>

#define MIN_MATCH_COUNT 5
#define NN_MATCH_RATIO 0.8
#define RANSAC_THRESH 2.5

struct BannerInfo
{
    cv::Mat image;
    QString name = "Banner";
    QString location = "";

    BannerInfo(){}
    BannerInfo(const BannerInfo &other)
    {
        this->name = other.name;
        this->location = other.location;
        this->image = other.image.clone();
    }
};

typedef struct
{
    double size; // Size
    double visible; // Visible
    double onscreen = 1.0; // On-screen
    double occlusion = 0.0; // Occlusion
    QString position = ""; // Position: Top,Bottom,Left,Right,Center
    double quality = 0.99; // Quality
    std::vector<cv::Point2f> corners;
}MatchInfo;

typedef struct
{
    BannerInfo bannerInfo;
    MatchInfo matchInfo;

    QString program = ""; // Program
    int hid; // Hit
    QString relation = ""; // Relation
    int frame; // Frame
    QString origin = "Automatic"; // Origin: Automatic, Manual
}MagellanHit;

static QStringList MatcherTypes = QStringList() << "FlannBased" << "BruteForce" << "BruteForce-L1" << "BruteForce-Hamming" << "BruteForce-Hamming(2)";

//static QStringList FeatureTypes = QStringList() << "SIFT" << "SURF" << "FREAK" << "DAISY" << "LATCH" << "LUCID";
static QStringList FeatureTypes = QStringList() << "SIFT" << "SURF" << "ORB" << "SIMPLE";

typedef struct
{
    int minMatchCount = MIN_MATCH_COUNT;
    double nnMatchRatio = NN_MATCH_RATIO;
    double ransacThrs = RANSAC_THRESH;
    int partPolicy = 2;
    double thrsCorrection = 0.0;
    bool binPreproc = false;
    double scale = 1.0;

    int mTypeIndex = 0;
    int fTypeIndex = 0;
    // SIFT
    double sift_contrastThrs = 0.02;
    double sift_edgeThrs = 15.0;
    // SURF
    double surf_hessThrs = 100.0; // 2000.0
    int surf_nOctaves = 4;
    bool surf_upright = true;
    bool surf_extended = false;
    // ORB
    double orb_scaleFactor = 1.25;
    int orb_nLevels = 8;
    int orb_edgeThrs = 5;
    int orb_wtak = 2;
    int orb_patchSize = 10;
    // SIMPLE
    double simple_matchThrs = 0.1;

    // add parameters here

}Parameters;

typedef struct
{
    QString workDir = "";
    QStringList fileNames;
    bool isVideo;
}AppState;

#endif // STRUCTURES_H
