#include <opencv2/opencv.hpp>
#include <iostream>

#include <fitsio.h>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
    fitsfile *fptr;
    char card[FLEN_CARD];
    int status = 0, nkeys, ii;

    if (argc != 2) {
        cout << "Please provide a file to open" << endl;
        return 0;
    }

    // Open the file
    fits_open_file(&fptr, argv[1], READONLY, &status);
    fits_get_hdrspace(fptr, &nkeys, NULL, &status);

    // List every meta data
    for (ii = 1; ii <= nkeys; ii++)
    {
        fits_read_record(fptr, ii, card, &status);
        printf("%s\n", card);
    }

    // Read every pixel, stretch resize and display
    int bitpix;
    int ret = fits_get_img_type(fptr, &bitpix, &status);
    cout << "Depth : " << bitpix << "bits" << endl;
    if (ret != 0) {
        cout << "Error: could not determine depth" << endl;
    }
    if (bitpix != 8) {
        cout << "Warning: Only 8 bits images have been tested" << endl;
    }

    int naxis;
    ret = fits_get_img_dim(fptr, &naxis, &status);
    if (ret != 0) {
        cout << "Error: could not determe how many channels are in the image" << endl;
    }
    cout << "Channels: " << naxis << endl;

    long naxes[3];
    ret = fits_get_img_size(fptr, naxis /*maxdim*/, naxes, &status);
    if (ret != 0) {
        cout << "Error: could not determe how many channels are in the image" << endl;
    }
    cout << "Size: " << naxes[0] << "x" << naxes[1] << "x" << naxes[2] << endl;

    int size_x = naxes[1], size_y = naxes[0];

    if (naxes[2] != 3) {
        cout<<"Error: only 3 channels images are supported, this one is "<<naxes<<endl;
        fits_close_file(fptr, &status);
        exit(1);
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

    
    Mat img(size_x /*x*/,size_y /*y*/, CV_8UC3 /*Assuming naxes[2] == 3*/, Scalar(0,0,0));
    
    uint8_t* pixelPtr = (uint8_t*)img.data;
    int cn = img.channels();
    Scalar_<uint8_t> bgrPixel;

    unsigned long long int count = 0;
    for (int i = 0; i < size_x*size_y; i++) {
        count += rarray[i];
    }
    float avg = count / ((float)size_x*size_y);

    for(int j = 0; j < img.cols; j++)
    {
        for(int i = 0; i < img.rows; i++)
        {   
            float r = 16/avg * rarray[i*img.cols + j];
            float g = 16/avg * garray[i*img.cols + j];
            float b = 16/avg * barray[i*img.cols + j];
            if (r > 255)
                r = 255;
            if (g > 255)
                r = 255;
            if (b > 255)
                r = 255;
            pixelPtr[i*img.cols*cn + j*cn + 0] = b;
            pixelPtr[i*img.cols*cn + j*cn + 1] = g;
            pixelPtr[i*img.cols*cn + j*cn + 2] = r;
            
        }
    }

    free(rarray);
    free(garray);
    free(barray);
    fits_close_file(fptr, &status);
    
    cv::resize(img, img, cv::Size(img.cols * 0.25,img.rows * 0.25), 0, 0);
    String windowName = "FIT viewer";
    namedWindow(windowName);
    imshow(windowName, img);
    //resizeWindow(windowName, 800, 600);
    waitKey(0);
    //destroyWindow(windowName); // Already done automatically
    //ReleaseImage(&img); // TODO: clean up memory


    if (status)
        fits_report_error(stderr, status);
    return (status);
}