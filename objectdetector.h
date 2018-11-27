#ifndef OBJECTDETECTOR_H
#define OBJECTDETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <vector>
#include "structures.h"

#define BLACK_STRIPES_THRS 20

class ObjectDetector
{
public:
    ObjectDetector();
    cv::Mat addQueryImage(cv::Mat scene, std::vector<cv::Point> points);
    cv::Mat getResultScene();
    void setMatchingParams(Parameters *params);
    void resetDrawScene(cv::Mat frame);
    std::vector<MatchInfo> getHits(){return mhits;}
    void setDebugMode(bool debugPrinting, bool debugDrawing = false)
    {
        this->debugPrinting = debugPrinting;
        this->debugDrawing = debugDrawing;
    }

    void computeQueryDescriptors(std::vector<cv::Mat> images);
    void computeSceneDescriptors(cv::Mat scene);
    void computeMatches(cv::Mat query, cv::Mat scene, int i);
    void autoDetectScaling(cv::Mat scene);

private:
    bool doMatching(cv::Mat query_image, cv::Mat scene_image);
    cv::Mat cutBlackStripes(cv::Mat source);
    cv::Mat preprocess(cv::Mat frame, bool isQuery = false);
    bool ratioCondition(std::vector<cv::Point2f> src, std::vector<cv::Point2f> dst);
    bool correctPoly(std::vector<cv::Point2f> poly);
    void excludePoly(cv::Mat &frame);
    void drawHits();
    void drawCorners(cv::Mat scene);
    void refineSceneCorners(cv::Mat scene);
    double backgroundValue(cv::Mat src);
    void addHit();
    void filterHits();
    void computeDescriptors(cv::Mat im, cv::Ptr<cv::Feature2D> detector,
                            std::vector<cv::KeyPoint> &kpts, cv::Mat &descs);
    bool simpleMatching(cv::Mat query_image, cv::Mat scene_image);
    cv::Ptr<cv::DescriptorMatcher> matcher;
    cv::Ptr<cv::Feature2D> detector_query;
    cv::Ptr<cv::Feature2D> detector_scene;
    std::vector<cv::KeyPoint> keypoints_query;
    cv::Mat descriptors_query;
    std::vector< cv::KeyPoint > keypoints_scene;
    cv::Mat descriptors_scene;
    // structures for multiple images matching
    std::vector<std::vector<cv::KeyPoint>> trainKeypoints;
    std::vector<cv::Mat> trainDescriptors;
    std::vector<std::vector<cv::KeyPoint>> sceneKeypoints;
    std::vector<cv::Mat> sceneDescriptors;
    std::vector<int> currTops;
    std::vector<int> currLefts;
    int partW, partH;

    cv::Mat draw_scene;
    QString object_name;

    std::vector<MatchInfo> mhits;

    std::vector<cv::Point2f> query_corners;
    std::vector<cv::Point2f> scene_corners;

    int blackH;
    double bVal;
    double thresh_query;
    int frame_square;
    int curr_top, curr_left;
    bool debugPrinting;
    bool debugDrawing;
    double decimation_factor;

//matching parameters
    Parameters *params;
};

#endif // OBJECTDETECTOR_H
