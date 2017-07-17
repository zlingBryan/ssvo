#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "frame.hpp"

namespace ssvo{

long int Frame::frameId = 0;

Frame::Frame(const cv::Mat& img, const double timestamp, cv::Mat& K, cv::Mat& dist_coef):
    id_(frameId++), timestamp_(timestamp), K_(K)
{
    assert(K.cols == 3 && K.rows == 3);
    fx = K.at<float>(0,0);
    fy = K.at<float>(1,1);
    cx = K.at<float>(0,2);
    cy = K.at<float>(1,2);

    assert(dist_coef.cols == 1 && dist_coef.rows == 4);
    k1 = dist_coef.at<float>(0);
    k2 = dist_coef.at<float>(1);
    p1 = dist_coef.at<float>(2);
    p2 = dist_coef.at<float>(3);

    pyr_levels_ = createPyramid(img, img_pyr_);
}

int Frame::createPyramid(const cv::Mat& img, std::vector<cv::Mat>& img_pyr, const uint16_t nlevels, const cv::Size min_size)
{
    assert(!img.empty());

    img_pyr.resize(nlevels);

    if(img.type() != CV_8UC1)
        cv::cvtColor(img, img_pyr[0], cv::COLOR_RGB2GRAY);
    else
        img.copyTo(img_pyr[0]);

    for(int i = 1; i < nlevels; ++i)
    {
        cv::Size size(round(img_pyr[i - 1].cols >> 1), round(img_pyr[i - 1].rows >> 1));

        if(size.height < min_size.height || size.width < min_size.width)
        {
            img_pyr.resize(i);
            return i;
        }

        cv::resize(img_pyr[i - 1], img_pyr[i], size, 0, 0, cv::INTER_LINEAR);
    }

    return nlevels;
}

int Frame::detectFeatures(FastDetector& detector)
{
    std::vector<cv::KeyPoint> kps_tracked;
    cv::KeyPoint::convert(pts_, kps_tracked);
    return detector.detectByImage(img_pyr_, kps_, kps_tracked);
}

}