#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

#include "fits.hpp"

#define MAX_STEPS 100 // Max "Size" of a star (in pixel)
#define MIN_DISTANCE_BETWEEN_STARS 10
#define PIXVAL_THRESHOLD 10 // number of time the average pixel value

FitImage dbg_img;

class Feature {
    public:
    cv::Point2f origin;
    cv::Point2f destination;
    cv::Point2f vector;
    void display(FitImage* img) {
        cv::circle(img->data, origin, 10, cv::Scalar(255, 0, 0), 5); // BGR
        cv::circle(dbg_img.data, destination, 10, cv::Scalar(255, 0, 127), 5);
        cv::line(dbg_img.data, origin, destination, cv::Scalar(0, 0, 255), 5);
    }
};

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

float getImgAveragePixel(cv::Mat &grayimg) {
    uint8_t* pixelPtr = (uint8_t*)grayimg.data;
    unsigned long long int sum = 0;
    for(int j = 0; j < grayimg.cols; j++)
    {
        for(int i = 0; i < grayimg.rows; i++)
        {   
            sum += pixelPtr[i*grayimg.cols + j]; // Gray only
        }
    }
    return (float)sum/(float)(grayimg.cols*grayimg.rows);
}

void listStars(std::vector<cv::Rect>* out_boxes, std::vector<cv::Point2f>* out_centers, cv::Mat &grayimg, float threshold) {
    uint8_t thrld = (uint8_t) threshold;
    std::cout << "star threshold is "<<(unsigned int)thrld<<std::endl;
    size_t single_pixel_star = 0;
    uint8_t* pixelPtr = (uint8_t*)grayimg.data;
    for(int j = 0; j < grayimg.cols; j++)
    {
        for(int i = 0; i < grayimg.rows; i++)
        {   
            if (pixelPtr[i*grayimg.cols + j] > thrld) {
                //pixelPtr[i*grayimg.cols + j] = 255; // Debug

                cv::Rect2f tmp = getStarBoundary(grayimg, j, i, thrld);

                if (tmp.width == 0 && tmp.height == 0) {
                    single_pixel_star ++;
                    continue;
                }

                cv::Point2f center = cv::Point2f(tmp.x + tmp.width/2, tmp.y + tmp.height/2);

                j += tmp.width + MIN_DISTANCE_BETWEEN_STARS;
                i += tmp.height + MIN_DISTANCE_BETWEEN_STARS;

                if (out_boxes != NULL)
                    out_boxes->push_back(tmp);
                if (out_centers != NULL)
                    out_centers->push_back(center);
            }
        }
    }
    if (out_boxes != NULL)
    std::cout<<out_boxes->size()<<" stars detected"<<std::endl;

    std::cout<<"Skipped single pixel star "<<single_pixel_star<<std::endl;
}

std::vector<Feature> matchStarsBruteForce(std::vector<cv::Point2f>* point1, std::vector<cv::Point2f>* pointref) {
    std::vector<Feature> movement_vector;

    
    for (std::vector<cv::Point2f>::iterator it = point1->begin(); it != point1->end(); it++) {
        float smallest_distance = 10000000;
        cv::Point2f* best_match;
        for (std::vector<cv::Point2f>::iterator it2 = pointref->begin(); it2 != pointref->end(); it2++) {
            float distance = (it->x - it2->x) * (it->x - it2->x) + (it->y - it2->y) * (it->y - it2->y);
            if (distance < smallest_distance) {
                best_match = (cv::Point2f*)&(*it2);
                smallest_distance = distance;
            }
        }
        if (smallest_distance < 10000000) {
            Feature tmp;
            tmp.origin = *it;
            tmp.destination = *best_match;
            tmp.vector = cv::Point2f(best_match->x - it->x, best_match->y - it->y);
            movement_vector.push_back(tmp);
        }
    }

    std::cout<<"Failed to match: "<<point1->size() - movement_vector.size()<< "stars" <<std::endl;

    return movement_vector;
}

cv::Point2f detectMotion(FitImage &im1, FitImage &im2) {
    im1.data.copyTo(dbg_img.data);

    dbg_img.sumWith(&im2);

    //dbg_img.create(im1.data.cols, im1.data.rows); // For blank debug
    dbg_img.contrast(30); // High streching

    cv::Mat im1Gray, im2Gray;
    cvtColor(im1.data, im1Gray, cv::COLOR_BGR2GRAY);
    cvtColor(im2.data, im2Gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Point2f> points1, points2;

    // Get average pixel value
    float avg1 = getImgAveragePixel(im1Gray);
    std::cout << "Average IMG1 is "<<avg1<<std::endl;

    float avg2 = getImgAveragePixel(im2Gray);
    std::cout << "Average IMG2 is "<<avg2<<std::endl;

    // Apply threshold and detect star's boundary when threshold is reached
    listStars(NULL, &points1, im1Gray, PIXVAL_THRESHOLD*avg1);

    listStars(NULL, &points2, im2Gray, PIXVAL_THRESHOLD*avg2);

    std::vector<Feature> features = matchStarsBruteForce(&points1, &points2);

    double sum_x = 0, sum_y = 0;
    for (std::vector<Feature>::iterator it = features.begin(); it != features.end(); it++) {
        sum_x += it->vector.x;
        sum_y += it->vector.y;
    }
    float avg_x = (float)sum_x / (float)features.size();
    float avg_y = (float)sum_y / (float)features.size();
    std::cout<<"Average before filtering x:"<<avg_x<<" y:"<<avg_y<<std::endl;

    // Filtering
    sum_x = 0, sum_y = 0;
    const float skip_threshold = 15.f;
    int skipped = 0;
    for (std::vector<Feature>::iterator it = features.begin(); it != features.end(); it++) {
        std::cout<<"Vector x:"<<it->vector.x<<" y:"<<it->vector.y<<std::endl;
        if (abs(it->vector.x - avg_x) > skip_threshold) {
            std::cout<<"x too high/low"<<std::endl;
            skipped++;
            continue;
        }
        else if (abs(it->vector.y - avg_y) > skip_threshold) {
            std::cout<<"y too high/low"<<std::endl;
            skipped++;
            continue;
        }
        it->display(&dbg_img);
        sum_x += it->vector.x;
        sum_y += it->vector.y;
    }
    avg_x = (float)sum_x / (float)features.size();
    avg_y = (float)sum_y / (float)features.size();
    std::cout<<"Average x:"<<avg_x<<" y:"<<avg_y<<std::endl;

    std::cout<<skipped<<"/" << features.size() << "vectors skipped"<<std::endl;

    cv::line(dbg_img.data, cv::Point(100, 100), cv::Point(100+avg_x, 100+avg_y), cv::Scalar(255, 255, 0), 5);

    cv::namedWindow("debug",cv::WindowFlags::WINDOW_NORMAL);
    cv::imshow("debug",dbg_img.data);
    cv::resizeWindow("debug", 1000,1000);
    cv::waitKey(0);
    return cv::Point2f(avg_x, avg_y); // TODO: return something usefull
}