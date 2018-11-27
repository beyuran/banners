#include "objectdetector.h"
#include <QString>

ObjectDetector::ObjectDetector()
{
#if CV_MAJOR_VERSION < 3
    /// Old OpenCV 2 code goes here.
    cv::OrbFeatureDetector detector(500, 1.8);
#else
    /// New OpenCV 3 code goes here.
    detector_query = cv::xfeatures2d::SIFT::create(0,3, 0.02, 10, 1.6);
    detector_scene = cv::xfeatures2d::SIFT::create(0,3, 0.02, 10, 1.6);
#endif
    matcher = cv::DescriptorMatcher::create("FlannBased");
    query_corners.assign(4,cv::Point2f(0,0));
    scene_corners.assign(4,cv::Point2f(0,0));
    blackH = 0;
    decimation_factor = 1.0;
}

void ObjectDetector::setMatchingParams(Parameters *params)
{
    this->params = params;
    detector_query.release();
    detector_scene.release();
    switch(params->fTypeIndex)
    {
    case 0:
        detector_query = cv::xfeatures2d::SIFT::create(0, 3, params->sift_contrastThrs, params->sift_edgeThrs, 1.6);
        detector_scene = cv::xfeatures2d::SIFT::create(0, 3, params->sift_contrastThrs, params->sift_edgeThrs, 1.6);
        break;
    case 1:
        detector_query = cv::xfeatures2d::SURF::create(params->surf_hessThrs, params->surf_nOctaves, 3, params->surf_extended, params->surf_upright);
        detector_scene = cv::xfeatures2d::SURF::create(params->surf_hessThrs, params->surf_nOctaves, 3, params->surf_extended, params->surf_upright);
        break;
    case 2:
        detector_query = cv::ORB::create(500, params->orb_scaleFactor, params->orb_nLevels, params->orb_edgeThrs, 0, params->orb_wtak, cv::ORB::HARRIS_SCORE, params->orb_patchSize);
        detector_scene = cv::ORB::create(500, params->orb_scaleFactor, params->orb_nLevels, params->orb_edgeThrs, 0, params->orb_wtak, cv::ORB::HARRIS_SCORE, params->orb_patchSize);
        break;
    case 3:
        // Test simple matching
        break;
//    case 3:
//        detector_query = cv::xfeatures2d::FREAK::create();
//        detector_scene = cv::xfeatures2d::FREAK::create();
//        break;
//    case 4:
//        detector_query = cv::xfeatures2d::DAISY::create();
//        detector_scene = cv::xfeatures2d::DAISY::create();
//        break;
//    case 5:
//        detector_query = cv::xfeatures2d::LATCH::create();
//        detector_scene = cv::xfeatures2d::LATCH::create();
//        break;
//    case 6:
//        detector_query = cv::xfeatures2d::LUCID::create(1, 2);
//        detector_scene = cv::xfeatures2d::LUCID::create(1, 2);
//        break;
    default:
        detector_query = cv::xfeatures2d::SIFT::create(0,3, params->sift_contrastThrs, params->sift_edgeThrs, 1.6);
        detector_scene = cv::xfeatures2d::SIFT::create(0,3, params->sift_contrastThrs, params->sift_edgeThrs, 1.6);
    };
    matcher.release();
    matcher = cv::DescriptorMatcher::create(MatcherTypes[params->mTypeIndex].toStdString());
    blackH = 0;
}

void ObjectDetector::resetDrawScene(cv::Mat frame)
{
    draw_scene = frame.clone();
}

void ObjectDetector::excludePoly(cv::Mat &frame)
{
    std::vector<cv::Point> corners;
    for (int i=0 ; i < scene_corners.size(); i++)
        corners.push_back(cv::Point(scene_corners[i].x / decimation_factor, scene_corners[i].y / decimation_factor));
    const cv::Point *pts = (const cv::Point*) cv::Mat(corners).data;
    int npts = cv::Mat(corners).rows;

    /// Exclude detected object from scene by filling with background
    //if (bVal < 150)
    //    cv::fillPoly(frame, &pts, &npts, 1, cv::Scalar(0, 0, 0), 8);
    //else
    //    cv::fillPoly(frame, &pts, &npts, 1, cv::Scalar(255, 255, 255), 8);
    cv::fillPoly(frame, &pts, &npts, 1, cv::Scalar(bVal, bVal, bVal), 8);
}

cv::Mat ObjectDetector::cutBlackStripes(cv::Mat source)
{
    cv::Mat result;
    if (blackH == 0)
    {
        uchar blackThresh = BLACK_STRIPES_THRS;
        int upperBnd = 0;
        int bottmBnd = 0;
        cv::Mat grey;

        cvtColor(source, grey, CV_BGR2GRAY);

        for (int i = 0; i < grey.rows; i++)
        {
            cv::Mat row = grey.row(i);
            uchar maxVal = 0;
            for (int j = 0; j < row.cols; j++)
            {
                uchar val = row.at<uchar>(j);
                if (val > maxVal)
                    maxVal = val;
            }
            if (maxVal > blackThresh)
            {
                upperBnd = i;
                break;
            }
        }

        for (int i = 0; i < grey.rows; i++)
        {
            cv::Mat row = grey.row(grey.rows - 1 - i);
            uchar maxVal = 0;
            for (int j = 0; j < row.cols; j++)
            {
                uchar val = row.at<uchar>(j);
                if (val > maxVal)
                    maxVal = val;
            }
            if (maxVal > blackThresh)
            {
                bottmBnd = i;
                break;
            }
        }

        blackH = std::max(upperBnd, bottmBnd);
        //blackH = std::min(blackH, source.rows / 4);
        blackH = std::min(blackH, 75);
    }
    result = cv::Mat(source, cv::Rect(0, blackH, source.cols, source.rows - 2 * blackH));
    return result;
}

cv::Mat ObjectDetector::preprocess(cv::Mat frame, bool isQuery)
{
    cv::Mat gray;
    cv::cvtColor(frame,gray,CV_BGR2GRAY);

    if(!params->binPreproc)
        return gray;

    if (isQuery)
    {
        thresh_query = cv::threshold(gray, gray, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
        thresh_query += params->thrsCorrection;
        if (debugPrinting)
            printf("Query binarization threshold = %.1f\n", thresh_query);
    }
    else
    {
        cv::threshold(gray, gray, thresh_query, 255, cv::THRESH_BINARY);
    }
    return gray;
}

bool ObjectDetector::ratioCondition(std::vector<cv::Point2f> src, std::vector<cv::Point2f> dst)
{
    double scaleW1 = cv::norm(dst[1] - dst[0]) / cv::norm(src[1] - src[0]);
    double scaleH1 = cv::norm(dst[2] - dst[1]) / cv::norm(src[2] - src[1]);
    double scaleW2 = cv::norm(dst[3] - dst[2]) / cv::norm(src[3] - src[2]);
    double scaleH2 = cv::norm(dst[0] - dst[3]) / cv::norm(src[0] - src[3]);

    //printf("%s: %.2f, %.2f, %.2f, %.2f\n", object_name.toStdString().data(),
    //       scaleW1, scaleH1, scaleW2, scaleH2);
    //printf("%.2f, %.2f, %.2f\n",
    //       scaleW1 / scaleH1, scaleH1 / scaleW2, scaleW2 / scaleH2);
    if (scaleW1 / scaleH1 < 2.0 && scaleW1 / scaleH1 > 0.4 &&
        scaleH1 / scaleW2 < 2.0 && scaleH1 / scaleW2 > 0.4 &&
        scaleW2 / scaleH2 < 2.0 && scaleW2 / scaleH2 > 0.4 )
        return true;
    else
        return false;
}

bool ObjectDetector::simpleMatching(cv::Mat query_image, cv::Mat scene_image)
{
    cv::Mat query_gray, scene_gray, match;
    cv::cvtColor(query_image, query_gray, CV_BGR2GRAY);
    cv::cvtColor(scene_image, scene_gray, CV_BGR2GRAY);
    cv::matchTemplate(scene_gray, query_gray, match, cv::TM_SQDIFF_NORMED);
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(match, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
    if (debugPrinting)
        printf("-- simpleMatching minVal: %.2f, (thrs: %.2f)\n",minVal, params->simple_matchThrs);
    if (minVal < params->simple_matchThrs)
    {
        cv::Point matchLoc = minLoc;
        scene_corners[0] = cv::Point(matchLoc.x,matchLoc.y);
        scene_corners[1] = cv::Point(matchLoc.x + query_image.cols, matchLoc.y);
        scene_corners[2] = cv::Point(matchLoc.x + query_image.cols, matchLoc.y + query_image.rows);
        scene_corners[3] = cv::Point(matchLoc.x, matchLoc.y + query_image.rows);
        return true;
    }
    return false;
}

bool ObjectDetector::doMatching(cv::Mat query_image_orig, cv::Mat scene_image_orig)
{
    cv::Mat debugWin;
    cv::Mat query_image, scene_image;
    query_image_orig.copyTo(query_image);
    scene_image_orig.copyTo(scene_image);

    if (decimation_factor < 1.0)
    {
        cv::resize(query_image, query_image, cv::Size(), decimation_factor, decimation_factor, cv::INTER_AREA);
        cv::resize(scene_image, scene_image, cv::Size(), decimation_factor, decimation_factor, cv::INTER_AREA);
    }
    query_corners[0] = cv::Point(0,0); query_corners[1] = cv::Point(query_image.cols, 0);
    query_corners[2] = cv::Point(query_image.cols, query_image.rows); query_corners[3] = cv::Point(0, query_image.rows);

    if (params->fTypeIndex == 3)
        return simpleMatching(query_image, scene_image);

    if(keypoints_scene.size() <= 1)
    {
        if (debugPrinting)
            printf("-- Condition failed: SCENE_DESC_EMPTY\n");
        return false;
    }
    std::vector<std::vector<cv::DMatch>> matches, good_matches;
    matcher->knnMatch(descriptors_query, descriptors_scene, matches, 2);

    for(unsigned i = 0; i < matches.size(); i++)
    {
        if(matches[i][0].distance < params->nnMatchRatio * matches[i][1].distance)
        {
            good_matches.push_back(matches[i]);
        }
    }
    if (debugPrinting)
        printf("-- All Matches: %d, Good Matches: %d\n",(int)matches.size(), (int)good_matches.size());

    if (good_matches.size() < params->minMatchCount)
    {
        if (debugPrinting)
            printf("-- Condition failed: MIN_MATCH_COUNT\n");
        return false;
    }
    //else printf("-- Condition passed: MIN_MATCH_COUNT\n");

    /// Localize the object
    std::vector<cv::Point2f> obj;
    std::vector<cv::Point2f> scene;
    for( int i = 0; i < good_matches.size(); i++ )
    {
        /// Get the keypoints from the good matches
        obj.push_back( keypoints_query[ good_matches[i][0].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i][0].trainIdx ].pt );
    }

    if (debugDrawing)
    {
        cv::drawMatches( query_image, keypoints_query, scene_image, keypoints_scene,
                         good_matches, debugWin, cv::Scalar::all(-1), cv::Scalar::all(-1),
                         std::vector<std::vector<char>>(),
                         cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
        cv::imshow( "Good Matches", debugWin );
        cv::waitKey(0);
    }

    cv::Mat inlinerMask;
    cv::Mat H = cv::findHomography( obj, scene, CV_RANSAC, params->ransacThrs, inlinerMask);

    if (H.empty())
    {
        if (debugPrinting)
            printf("-- Condition failed: FIND_HOMOGRAPHY\n");
        return false;
    }
    //else printf("-- Condition passed: FIND_HOMOGRAPHY\n");

    int inliners_count = 0;
    std::vector<cv::Point2f> obj_2;
    std::vector<cv::Point2f> scene_2;
    for(unsigned i = 0; i < good_matches.size(); i++) {
        if(inlinerMask.at<uchar>(i)) {
            inliners_count++;
            obj_2.push_back(obj[i]);
            scene_2.push_back(scene[i]);
        }
    }
    if (debugPrinting)
        printf("-- Inliners %d\n", inliners_count);

    cv::perspectiveTransform( query_corners, scene_corners, H);

    cv::Rect sbox = cv::boundingRect(scene_corners);
    if (!(0 <= sbox.x && sbox.x + sbox.width <= scene_image.cols &&
        0 <= sbox.y && sbox.y + sbox.height <= scene_image.rows))
    {
        if (debugPrinting)
            printf("-- Condition failed: BOUNDARIES\n");

        return false;
        //refineSceneCorners(scene_image);
    }
    //else printf("-- Condition passed: BOUNDARIES\n");

    //if (debugDrawing)
    //    drawCorners(scene_image);

    if (!correctPoly(scene_corners))
    {
        if (debugPrinting)
            printf("-- Condition failed: CORRECT_POLYGON\n");
        return false;
    }
    //else printf("-- Condition passed: CORRECT_POLYGON\n");

    if (debugDrawing)
    {
        std::vector<cv::KeyPoint> matched1, matched2;
        for(unsigned i = 0; i < matches.size(); i++) {
            if(matches[i][0].distance < params->nnMatchRatio * matches[i][1].distance) {
                matched1.push_back(keypoints_query[matches[i][0].queryIdx]);
                matched2.push_back(keypoints_scene[matches[i][0].trainIdx]);
            }
        }

        std::vector<cv::KeyPoint> inliers1, inliers2;
        std::vector<cv::DMatch> inlier_matches;

        for(unsigned i = 0; i < matched1.size(); i++) {
            if(inlinerMask.at<uchar>(i)) {
                int new_i = static_cast<int>(inliers1.size());
                inliers1.push_back(matched1[i]);
                inliers2.push_back(matched2[i]);
                inlier_matches.push_back(cv::DMatch(new_i, new_i, 0));
            }
        }
        cv::drawMatches(query_image, inliers1, scene_image, inliers2,
                      inlier_matches, debugWin,
                      cv::Scalar(255, 0, 0), cv::Scalar(255, 0, 0));
        cv::imshow( "Inliners matches", debugWin );
        cv::waitKey(0);
    }

    if (!ratioCondition(query_corners, scene_corners))
    {
        if (debugPrinting)
            printf("-- Condition failed: SIDE_RATIO\n");
        return false;
    }
    //else printf("-- Condition passed: SIDE_RATIO\n");

    return true;
}

void ObjectDetector::drawHits()
{
    for (int i = 0; i < mhits.size(); i++)
    {
        std::vector<cv::Point> corners;
        for (int j = 0 ; j < mhits[i].corners.size(); j++)
            corners.push_back(cv::Point(mhits[i].corners[j].x, mhits[i].corners[j].y));

        const cv::Point *pts = (const cv::Point*) cv::Mat(corners).data;
        int npts = cv::Mat(corners).rows;
        cv::polylines(draw_scene, &pts,&npts, 1, true, cv::Scalar(0,255,0));
        if (object_name != "")
        {
            cv::Rect rect = cv::boundingRect(mhits[i].corners);
            cv::putText(draw_scene, object_name.toStdString(), rect.tl(), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(50,50,200), 1);
        }
    }
}

void ObjectDetector::drawCorners(cv::Mat scene)
{
    cv::Mat debugWin = scene.clone();
    std::vector<cv::Point> corners;
    for (int i=0 ; i < scene_corners.size(); i++)
        corners.push_back(cv::Point(scene_corners[i].x, scene_corners[i].y));

    const cv::Point *pts = (const cv::Point*) cv::Mat(corners).data;
    int npts = cv::Mat(corners).rows;
    cv::polylines(debugWin, &pts,&npts, 1, true, cv::Scalar(0,255,0));
    cv::imshow("Result corners", debugWin);
    cv::waitKey(0);
}

cv::Mat ObjectDetector::addQueryImage(cv::Mat scene, std::vector<cv::Point> points)
{
    cv::Mat roi;
    scene.copyTo(roi);
    const cv::Point* elementPoints[1] = { &points[0] };
    int numberOfPoints = (int)points.size();

    /// Compute background value
    cv::Mat rect_banner = cv::Mat(scene, cv::boundingRect(points));
    bVal = backgroundValue(rect_banner);
    /// Cut polygonal banner from scene
    cv::Mat stencil = cv::Mat::zeros(scene.rows, scene.cols, scene.type());
    cv::Mat stencil2 = cv::Mat::zeros(scene.rows, scene.cols, scene.type());
    //if (bVal < 150) /// if dark background
    //{
    //    printf("Dark background: %.0f\n", bVal);
    //    cv::fillPoly(stencil, elementPoints, &numberOfPoints, 1, cv::Scalar (255, 255, 255), 8);
    //    cv::bitwise_and(scene, stencil, roi);
    //}else /// if light background
    //{
    //    printf("Light background: %.0f\n", bVal);
    //    cv::bitwise_not(stencil,stencil);
    //    cv::fillPoly(stencil, elementPoints, &numberOfPoints, 1, cv::Scalar (0, 0, 0), 8);
    //    cv::bitwise_or(scene, stencil, roi);
    //}
    cv::fillPoly(stencil, elementPoints, &numberOfPoints, 1, cv::Scalar (255, 255, 255), 8);
    cv::bitwise_and(scene, stencil, stencil);   
    stencil2 = cv::Scalar(bVal, bVal, bVal);
    cv::fillPoly(stencil2, elementPoints, &numberOfPoints, 1, cv::Scalar (0, 0, 0), 8);
    cv::bitwise_or(stencil, stencil2, roi);
    cv::Mat banner = cv::Mat(roi, cv::boundingRect(points));

    return banner;
}

double ObjectDetector::backgroundValue(cv::Mat src)
{
    cv::Mat src_gray;
    cv::cvtColor(src, src_gray, CV_BGR2GRAY);

    /// Establish the number of bins
    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };
    bool uniform = true; bool accumulate = false;

    /// Compute the histograms
    cv::Mat v_hist;
    cv::calcHist( &src_gray, 1, 0, cv::Mat(), v_hist, 1, &histSize, &histRange, uniform, accumulate );

    double vmax = v_hist.at<float>(0);
    double ci_max = 0;
    for( int i = 1; i < histSize; i++ )
    {
        if (v_hist.at<float>(i) > vmax)
        {
            ci_max = i;
            vmax = v_hist.at<float>(i);
        }
    }
    //printf("Background value: %.0f\n", ci_max);
    return ci_max;
}

cv::Mat ObjectDetector::getResultScene()
{
    QString text = "Draw result scene";
    cv::putText(draw_scene, text.toStdString(), cv::Point(100,50), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(50,170,50), 2);
    return draw_scene;
}

void ObjectDetector::refineSceneCorners(cv::Mat scene)
{
    for (int i = 0; i < scene_corners.size(); i++)
    {
        if (scene_corners[i].x < 0)
            scene_corners[i].x = 0;
        if (scene_corners[i].y < 0)
            scene_corners[i].y = 0;
        if (scene_corners[i].x >= scene.cols)
            scene_corners[i].x = scene.cols - 1;
        if (scene_corners[i].y >= scene.rows)
            scene_corners[i].y = scene.rows - 1;
    }
}

void ObjectDetector::addHit()
{
    MatchInfo mhit;

    mhit.size = cv::contourArea(scene_corners) / (double) frame_square;
    mhit.visible = mhit.size;

    for (int i=0 ; i < scene_corners.size(); i++)
    {
        double x = scene_corners[i].x / decimation_factor + curr_left;
        double y = scene_corners[i].y / decimation_factor + curr_top + blackH ;
        mhit.corners.push_back(cv::Point(x, y));
    }

    mhits.push_back(mhit);
}

void ObjectDetector::filterHits()
{
    bool stop = false;
    std::vector<std::vector<MatchInfo>::iterator> to_erase;
    for (auto i = mhits.begin(); i != mhits.end(); ++i)
    {
        auto j = i + 1;
        while (j != mhits.end())
        {
            cv::Rect A = cv::boundingRect(i->corners);
            cv::Rect B = cv::boundingRect(j->corners);
            if ((A & B).area() > 0.5 * std::min(A.area(), B.area()))
            {
                if (A.area() < B.area())
                {
                   i->corners = j->corners;
                   i->size = j->size;
                   i->visible = j->visible;
                }
                j = mhits.erase(j);
                if (j == mhits.end())
                {
                    stop = true;
                    break;
                }
                //printf("Double matching\n");
            }else
            {
                ++j;
            }
        }
    }
}

bool ObjectDetector::correctPoly(std::vector<cv::Point2f> poly)
{
    //The contour must be without self-intersections and convex.
    bool hasIntersection = false;
    cv::Point3f v11, v12, v21, v22, prod1, prod2, prod3, prod4, cut1, cut2;
    v11.x = poly[0].x; v11.y = poly[0].y; v11.z = 0.0;
    v12.x = poly[1].x; v12.y = poly[1].y; v12.z = 0.0;
    v21.x = poly[2].x; v21.y = poly[2].y; v21.z = 0.0;
    v22.x = poly[3].x; v22.y = poly[3].y; v22.z = 0.0;

    cut1 = v12 - v11;
    cut2 = v22 - v21;
    prod1 = cut1.cross(v21 - v11);
    prod2 = cut1.cross(v22 - v11);
    prod3 = cut2.cross(v11 - v21);
    prod4 = cut2.cross(v12 - v21);

    if (prod1.z * prod2.z > 0 || prod1.z == 0 || prod2.z == 0 ||
        prod3.z * prod4.z > 0 || prod3.z == 0 || prod4.z == 0)
        hasIntersection = false;
    else
        hasIntersection = true;

    if (hasIntersection)
        return false;

    return cv::isContourConvex(poly);
}

void ObjectDetector::computeDescriptors(cv::Mat im, cv::Ptr<cv::Feature2D> detector,
                        std::vector<cv::KeyPoint> &kpts, cv::Mat &descs)
{
    cv::Mat gray;
    cv::cvtColor(im, gray, CV_BGR2GRAY); // preprocess
    if (decimation_factor < 1.0)
        cv::resize(gray, gray, cv::Size(), decimation_factor, decimation_factor, cv::INTER_AREA);
    detector->detect(gray, kpts);
    detector->compute(gray, kpts, descs);
    if (debugDrawing)
    {
        cv::Mat debugWin;
        cv::drawKeypoints(gray, kpts, debugWin);
        cv::imshow("Query keypoints", debugWin);
        cv::waitKey(0);
    }
}

void ObjectDetector::computeQueryDescriptors(std::vector<cv::Mat> images)
{
    if (params->fTypeIndex == 3)
        return;

    trainKeypoints.clear();
    trainDescriptors.clear();
    for (int i = 0; i < images.size(); i++)
    {
        computeDescriptors(images[i], detector_query, keypoints_query, descriptors_query);
        if (debugPrinting)
            printf("Create query descriptor: %d pts\n",keypoints_query.size());

        trainKeypoints.push_back(keypoints_query);
        trainDescriptors.push_back(descriptors_query);
    }
}

void ObjectDetector::computeSceneDescriptors(cv::Mat scene)
{
    sceneKeypoints.clear();
    sceneDescriptors.clear();
    currTops.clear();
    currLefts.clear();

    cv::Mat scene_cut;
    scene_cut = cutBlackStripes(scene);

    int sc_w = scene_cut.cols;
    int sc_h = scene_cut.rows;

    switch(params->partPolicy)
    {
    case 0:
        partH = sc_h;
        partW = sc_w;
        break;
    case 1:
        partH = sc_h / 2;
        partW = sc_w;
        break;
    case 2:
        partH = sc_h / 3;
        partW = sc_w / 2;
        break;
    case 3:
        partH = sc_h / 3;
        partW = sc_w / 3;
        break;
    default:
        partH = sc_h;
        partW = sc_w;
    };

    curr_top = 0;
    curr_left = 0;

    while (curr_top + partH <= sc_h)
    {
        cv::Mat scene_part = cv::Mat(scene_cut, cv::Rect(curr_left, curr_top, partW, partH)).clone();
        currTops.push_back(curr_top);
        currLefts.push_back(curr_left);
        if (params->fTypeIndex != 3)
        {
            computeDescriptors(scene_part, detector_scene, keypoints_scene, descriptors_scene);
            sceneKeypoints.push_back(keypoints_scene);
            sceneDescriptors.push_back(descriptors_scene);
        }
        curr_left += partW / 2;
        if (curr_left + partW > sc_w)
        {
            curr_left = 0;
            curr_top += partH / 2;
        }
    }
}

void ObjectDetector::computeMatches(cv::Mat query, cv::Mat scene, int i)
{
    frame_square = scene.cols * scene.rows;
    mhits.clear();

    cv::Mat scene_cut;
    scene_cut = cutBlackStripes(scene);

    if (params->fTypeIndex != 3)
    {
        keypoints_query = trainKeypoints[i];
        descriptors_query = trainDescriptors[i];
        if (keypoints_query.size() <= 1)
        {
            printf("-- Query descriptor is empty!\n");
            return;
        }
    }
    for (int i = 0; i < currTops.size(); i++)
    {
        curr_top = currTops[i];
        curr_left = currLefts[i];
        cv::Mat scene_part;

        scene_part = cv::Mat(scene_cut, cv::Rect(curr_left, curr_top, partW, partH)).clone();

        int match_on_scene = 0;
        if (params->fTypeIndex != 3)
        {
            keypoints_scene = sceneKeypoints[i];
            descriptors_scene = sceneDescriptors[i];
        }
        while(doMatching(query, scene_part))
        {
            if (debugPrinting)
                std::cout << "Object detected\n";
            addHit();
            excludePoly(scene_part);
            if (params->fTypeIndex != 3)
                computeDescriptors(scene_part, detector_scene, keypoints_scene, descriptors_scene);
            match_on_scene++;
            if (match_on_scene > 4)
            {
                if (debugPrinting)
                    std::cout << "Infinite loop?!\n";
                break;
            }
        }
    }

    filterHits();
    if (!draw_scene.empty())
        drawHits();
}

void ObjectDetector::autoDetectScaling(cv::Mat scene)
{
    // To fit matching process into memory for large frames
    frame_square = scene.cols * scene.rows;
    decimation_factor = std::min(1.0, sqrt(500000.0 / frame_square)); // 1000000.0
    //if (debugPrinting)
    {
        std::cout << "Scale factor: " << decimation_factor << std::endl;
    }
}
