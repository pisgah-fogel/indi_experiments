#pragma once
#include <iostream>
#include <chrono>
#include <fitsio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

class FitImage
{
    public:
    void display() {
        cv::Mat tmp;
        cv::resize(data, tmp, cv::Size(data.cols * 0.25,data.rows * 0.25), 0, 0);
        std::string windowName = "FIT viewer";
        cv::namedWindow(windowName);
        cv:imshow(windowName, tmp);
        cv::waitKey(0); // TODO: add an option
    }
    bool open(std::string filename) {
        fitsfile *fptr;
        char card[FLEN_CARD];
        int status = 0, nkeys, ii;

        // Open the file
        auto begin = std::chrono::high_resolution_clock::now();
        fits_open_file(&fptr, filename.c_str(), READONLY, &status);
        fits_get_hdrspace(fptr, &nkeys, NULL, &status);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Open file: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << "ns" << std::endl;

        // List every meta data
        begin = std::chrono::high_resolution_clock::now();
        for (ii = 1; ii <= nkeys; ii++)
        {
            fits_read_record(fptr, ii, card, &status);
            printf("%s\n", card);
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "Read META: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << "ns" << std::endl;
    
        // Read every pixel, stretch resize and display
        int bitpix;
        int ret = fits_get_img_type(fptr, &bitpix, &status);
        std::cout << "Depth : " << bitpix << "bits" << std::endl;
        if (ret != 0) {
            std::cout << "Error: could not determine depth" << std::endl;
            fits_close_file(fptr, &status);
            return false;
        }
        if (bitpix != 8) {
            std::cout << "Warning: Only 8 bits images have been tested" << std::endl;
            fits_close_file(fptr, &status);
            return false;
        }

        int naxis;
        ret = fits_get_img_dim(fptr, &naxis, &status);
        if (ret != 0) {
            std::cout << "Error: could not determe how many channels are in the image" << std::endl;
            fits_close_file(fptr, &status);
            return false;
        }
        std::cout << "Channels: " << naxis << std::endl;

        long naxes[3];
        ret = fits_get_img_size(fptr, naxis /*maxdim*/, naxes, &status);
        if (ret != 0) {
            std::cout << "Error: could not determe how many channels are in the image" << std::endl;
            fits_close_file(fptr, &status);
            return false;
        }
        std::cout << "Size: " << naxes[0] << "x" << naxes[1] << "x" << naxes[2] << std::endl;

        int size_x = naxes[0], size_y = naxes[1];

        if (naxes[2] != 3) {
            std::cout<<"Error: only 3 channels images are supported, this one is "<<naxes<<std::endl;
            fits_close_file(fptr, &status);
            return false;
        }

        unsigned char *rarray = (unsigned char *)malloc(size_x*size_y); // Red channel
        unsigned char *garray = (unsigned char *)malloc(size_x*size_y); // Green channel
        unsigned char *barray = (unsigned char *)malloc(size_x*size_y); // Blue channel
        long fpixel [3]; // Size: NAXIS
        // fpixel[0] goes from 1 to NAXIS1
        // fpixel[1] goes from 1 to NAXIS2
        int anynul;
        fpixel[0] = fpixel[1] = 1;
        fpixel[2] = 1;
        ret = ffgpxv(fptr, TBYTE, fpixel, 
                    size_x*size_y, NULL, rarray, 
                    &anynul, &status);
        fpixel[0] = fpixel[1] = 1;
        fpixel[2] = 2;
        ret = ffgpxv(fptr, TBYTE, fpixel, 
                    size_x*size_y, NULL, garray, 
                    &anynul, &status);
        fpixel[0] = fpixel[1] = 1;
        fpixel[2] = 3;
        ret = ffgpxv(fptr, TBYTE, fpixel, 
                    size_x*size_y, NULL, barray, 
                    &anynul, &status);

        if (status)
        fits_report_error(stderr, status);

        data = cv::Mat(size_x /*x*/,size_y /*y*/, CV_8UC3 /*Assuming naxes[2] == 3*/, cv::Scalar(0,0,0));
        
        uint8_t* pixelPtr = (uint8_t*)data.data;
        int cn = data.channels();
        cv::Scalar_<uint8_t> bgrPixel;

        unsigned long long int count = 0;
        for (int i = 0; i < size_x*size_y; i++) {
            count += rarray[i];
        }
        float avg = count / ((float)size_x*size_y);

        for(int j = 0; j < data.cols; j++)
        {
            for(int i = 0; i < data.rows; i++)
            {   
                pixelPtr[i*data.cols*cn + j*cn + 0] = 10/avg * barray[j*data.rows + i];
                pixelPtr[i*data.cols*cn + j*cn + 1] = 10/avg * garray[j*data.rows + i];
                pixelPtr[i*data.cols*cn + j*cn + 2] = 10/avg * rarray[j*data.rows + i];
                
            }
        }

        free(rarray);
        free(garray);
        free(barray);
        fits_close_file(fptr, &status);
        return true;
    }
    cv::Mat data;
    private:
};