#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

#include <fitsio.h>
#include "fits.hpp"

using namespace cv;

const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 0.05f;

// im2: reference image
Point2f alignImages(Mat &im1, Mat &im2)
{
    // Convert images to grayscale
    Mat im1Gray, im2Gray;
    cvtColor(im1, im1Gray, cv::COLOR_BGR2GRAY);
    cvtColor(im2, im2Gray, cv::COLOR_BGR2GRAY);

    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
    orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
    orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);

    // Match features.
    std::vector<DMatch> matches;
    // BruteForce-Hamming does not work well
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce");
    matcher->match(descriptors1, descriptors2, matches, Mat());

    // Sort matches by score
    std::sort(matches.begin(), matches.end());

    // Remove not so good matches
    const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    matches.erase(matches.begin() + numGoodMatches, matches.end());

    // Draw top matches
    Mat imMatches;
    drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
    cv::resize(imMatches, imMatches, cv::Size(imMatches.cols * 0.25, imMatches.rows * 0.25), 0, 0);
    //imwrite("matches.jpg", imMatches);
    std::string windowName = "Matching";
    cv::namedWindow(windowName);
cv:
    imshow(windowName, imMatches);
    waitKey(0);

    // Extract location of good matches
    std::vector<Point2f> points1, points2;
    float avg_dev_x = 0;
    float avg_dev_y = 0;

    for (size_t i = 0; i < matches.size(); i++)
    {
        Point2f a = keypoints1[matches[i].queryIdx].pt;
        Point2f b = keypoints2[matches[i].trainIdx].pt;
        points1.push_back(a);
        points2.push_back(b);
        avg_dev_x += a.x - b.x;
        avg_dev_y += a.y - b.y;
    }
    avg_dev_x /= (float)matches.size();
    avg_dev_y /= (float)matches.size();
    std::cout << " Average deviation " << avg_dev_x << " x " << avg_dev_y << std::endl;
    // Find homography
    //h = findHomography( points1, points2, RANSAC );

    // Use homography to warp image
    //warpPerspective(im1, im1Reg, h, im2.size());
    return Point2f(avg_dev_x, avg_dev_y);
}

int main(int argc, char *argv[])
{
    FitImage a, b;

    if (argc != 3)
    {
        std::cout << "Please provide 2 filenames" << std::endl;
        return 1;
    }
    a.open(argv[1]);
    b.open(argv[2]);
    alignImages(a.data, b.data);
    return 0;
}