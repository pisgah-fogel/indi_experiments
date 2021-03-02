#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

#include "fits.hpp"

#define MAX_STEPS 100 // Max "Size" of a star (in pixel)
#define MIN_DISTANCE_BETWEEN_STARS 10

cv::Rect2f getStarBoundary(cv::Mat &grayimg, int x, int y, unsigned int thrld) {
    cv::Rect2f result;
    uint8_t* pixelPtr = (uint8_t*)grayimg.data;

    // TODO: check tmpx < width >= 0 (+ same for tmpy)
    // Find top (y small)
    int tmpx = x, tmpy = y;
    for (size_t steps = 0; steps < MAX_STEPS; steps ++) {
        if (pixelPtr[(tmpy-1)*grayimg.cols + tmpx] > thrld) {
            tmpy -= 1;
        }
        else if (pixelPtr[(tmpy-1)*grayimg.cols + tmpx-1] > thrld) {
            tmpy -= 1;
            tmpx -= 1;
        }
        else if (pixelPtr[(tmpy-1)*grayimg.cols + tmpx+1] > thrld) {
            tmpy -= 1;
            tmpx += 1;
        }
        else
            break;
    }
    result.y = tmpy;

    tmpx = x;
    tmpy = y;
    // Find bottom (y big)
    for (size_t steps = 0; steps < MAX_STEPS; steps ++) {
        if (pixelPtr[(tmpy+1)*grayimg.cols + tmpx] > thrld) {
            tmpy += 1;
        }
        else if (pixelPtr[(tmpy+1)*grayimg.cols + tmpx-1] > thrld) {
            tmpy += 1;
            tmpx -= 1;
        }
        else if (pixelPtr[(tmpy+1)*grayimg.cols + tmpx+1] > thrld) {
            tmpy += 1;
            tmpx += 1;
        }
        else
            break;
    }
    result.height = tmpy - result.y;

    tmpx = x;
    tmpy = y;
    // Find left (x small)
    for (size_t steps = 0; steps < MAX_STEPS; steps ++) {
        if (pixelPtr[(tmpy)*grayimg.cols + tmpx-1] > thrld) {
            tmpx -= 1;
        }
        else if (pixelPtr[(tmpy-1)*grayimg.cols + tmpx-1] > thrld) {
            tmpx -= 1;
            tmpy -= 1;
        }
        else if (pixelPtr[(tmpy+1)*grayimg.cols + tmpx-1] > thrld) {
            tmpx -= 1;
            tmpy += 1;
        }
        else
            break;
    }
    result.x = tmpx;

    tmpx = x;
    tmpy = y;
    // Find right (x big)
    for (size_t steps = 0; steps < MAX_STEPS; steps ++) {
        if (pixelPtr[(tmpy)*grayimg.cols + tmpx+1] > thrld) {
            tmpx += 1;
        }
        else if (pixelPtr[(tmpy-1)*grayimg.cols + tmpx+1] > thrld) {
            tmpx += 1;
            tmpy -= 1;
        }
        else if (pixelPtr[(tmpy+1)*grayimg.cols + tmpx+1] > thrld) {
            tmpx += 1;
            tmpy += 1;
        }
        else
            break;
    }
    result.width = tmpx - result.x;

    std::cout<<"Star x:"<<result.x<<" y:"<<result.y<<" w"<<result.width<<" h"<<result.height<<std::endl;

    return result;
}

cv::Point2f detectStars(FitImage &im1, FitImage &im2) {
    cv::Mat im1Gray, im2Gray;
    cvtColor(im1.data, im1Gray, cv::COLOR_BGR2GRAY);
    cvtColor(im2.data, im2Gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Point2f> points1, points2;

    // Get average pixel value
    uint8_t* pixelPtr = (uint8_t*)im1Gray.data;
    unsigned long long int sum = 0;
    for(int j = 0; j < im1Gray.cols; j++)
    {
        for(int i = 0; i < im1Gray.rows; i++)
        {   
            sum += pixelPtr[i*im1Gray.cols + j]; // Gray only
        }
    }
    float avg = (float)sum/(float)(im1Gray.cols*im1Gray.rows);

    // Apply threshold and detect star's boundary when threshold is reached
    uint8_t thrld = 10*avg;
    unsigned int count = 0;
    std::vector<cv::Rect> stars;
    std::cout << "Average pixel value is "<<avg<<std::endl;
    std::cout << "star threshold is "<<(unsigned int)thrld<<std::endl;
    for(int j = 0; j < im1Gray.cols; j++)
    {
        for(int i = 0; i < im1Gray.rows; i++)
        {   
            if (pixelPtr[i*im1Gray.cols + j] > thrld) {
                count++;
                //pixelPtr[i*im1Gray.cols + j] = 255; // Debug

                cv::Rect2f tmp = getStarBoundary(im1Gray, j, i, thrld);

                j += tmp.width + MIN_DISTANCE_BETWEEN_STARS;
                i += tmp.height + MIN_DISTANCE_BETWEEN_STARS;

                stars.push_back(tmp);
                // TODO: Save star position
                // TODO plot
            }
        }
    }
    std::cout<<count<<" pixels above threshold"<<std::endl;
    std::cout<<stars.size()<<" stars detected"<<std::endl;

    // Debugging threshold
    //cv:imshow("threshold",im1Gray);
    //cv::waitKey(0);

    //Debug star gazing
    cv::Mat debugimg;
    im1Gray.copyTo(debugimg);
    for (std::vector<cv::Rect>::iterator it = stars.begin(); it != stars.end(); it++)
        cv::rectangle(debugimg,*it, cv::Scalar(255));
    cv::namedWindow("stars",cv::WindowFlags::WINDOW_NORMAL);
    cv::imshow("stars",debugimg);
    cv::resizeWindow("stars", 600,800);
    cv::waitKey(0);

    // TODO get star center

    // TODO draw circles

}