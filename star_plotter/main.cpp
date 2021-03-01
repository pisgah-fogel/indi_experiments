#include <opencv2/opencv.hpp>
#include <iostream>

#include <fitsio.h>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
    fitsfile *fptr;
    char card[FLEN_CARD];
    int status = 0, nkeys, ii; /* MUST initialize status */

    fits_open_file(&fptr, argv[1], READONLY, &status);
    fits_get_hdrspace(fptr, &nkeys, NULL, &status);

    for (ii = 1; ii <= nkeys; ii++)
    {
        fits_read_record(fptr, ii, card, &status); /* read keyword */
        printf("%s\n", card);
    }
    printf("END\n\n"); /* terminate listing with END */

    // Alternative: Use fits_get_img_param instead

    int bitpix;
    int ret = fits_get_img_type(fptr, &bitpix, &status);
    cout << "bitpix: " << bitpix << " ("<<ret<<")"<< endl;

    int naxis;
    ret = fits_get_img_dim(fptr, &naxis, &status);
    cout << "dim: naxis: " << naxis << " ("<<ret<<")"<< endl;

    long naxes[3];
    ret = fits_get_img_size(fptr, naxis /*maxdim*/, naxes, &status);
    cout << "size: naxes: " << naxes[0] << "x" << naxes[1] << "x" << naxes[2] << " ("<<ret<<")"<< endl;

    int size_x = 4272, size_y = 2848;

    unsigned char *rarray = (unsigned char *)malloc(size_x*size_y); // TODO
    unsigned char *garray = (unsigned char *)malloc(size_x*size_y); // TODO
    unsigned char *barray = (unsigned char *)malloc(size_x*size_y); // TODO
    long fpixel [3]; // Size: NAXIS
    // fpixel0 goes from 1 to NAXIS1
    // fpixel1 goes from 1 to NAXIS2
    //long inc [] = {2, 1, 0};
    //long lpixel [] = {100, 100};
    int anynul;
    //fits_read_pix
    fpixel[0] = fpixel[1] = 1;
    fpixel[2] = 1;
    ret = ffgpxv(fptr, TBYTE, fpixel, 
                  size_x*size_y, NULL, rarray, 
                  &anynul, &status);
    cout<< "rarray[0] " << (int)rarray[0] <<endl;
    cout<< "rarray[100x100] " << (int)rarray[100*100] <<endl;
    fpixel[0] = fpixel[1] = 1;
    fpixel[2] = 2;
    ret = ffgpxv(fptr, TBYTE, fpixel, 
                  size_x*size_y, NULL, garray, 
                  &anynul, &status);
    cout<< "garray[0] " << (int)garray[0] <<endl;
    cout<< "garray[100x100] " << (int)garray[100*100] <<endl;
    fpixel[0] = fpixel[1] = 1;
    fpixel[2] = 3;
    ret = ffgpxv(fptr, TBYTE, fpixel, 
                  size_x*size_y, NULL, barray, 
                  &anynul, &status);
    cout<< "barray[0] " << (int)barray[0] <<endl;
    cout<< "barray[100x100] " << (int)barray[100*100] <<endl;

    
    // TODO: create opencv image
    Mat img(size_x /*x*/,size_y /*y*/, CV_8UC3 /*Assuming naxes[2] == 3*/, Scalar(0,0,0));
    
    
    // Accessing the pixel buffer, assuming it is 8 bits
    uint8_t* pixelPtr = (uint8_t*)img.data;
    int cn = img.channels();
    Scalar_<uint8_t> bgrPixel;

    for(int j = 0; j < img.cols; j++)
    {
        for(int i = 0; i < img.rows; i++)
        {
            //bgrPixel.val[0] = pixelPtr[i*img.cols*cn + j*cn + 0]; // B
            //bgrPixel.val[1] = pixelPtr[i*img.cols*cn + j*cn + 1]; // G
            //bgrPixel.val[2] = pixelPtr[i*img.cols*cn + j*cn + 2]; // R
            pixelPtr[i*img.cols*cn + j*cn + 0] = barray[j*img.rows + i];
            pixelPtr[i*img.cols*cn + j*cn + 1] = garray[j*img.rows + i];
            pixelPtr[i*img.cols*cn + j*cn + 2] = rarray[j*img.rows + i];
            
        }
    }

    free(rarray);
    free(garray);
    free(barray);
    fits_close_file(fptr, &status);


    String windowName = "FIT viewer"; //Name of the window
    namedWindow(windowName); // Create a window
    imshow(windowName, img); // Show our image inside the created window.
    waitKey(0); // Wait for any keystroke in the window
    destroyWindow(windowName); //destroy the created window


    //ReleaseImage(&img);








    

    if (status) /* print any error messages */
        fits_report_error(stderr, status);
    return (status);
}

/*
int main(int argc, char** argv)
{
    // Read the image file
    // 16bits: IMREAD_ANYCOLOR | IMREAD_ANYDEPTH
    Mat image = imread("/home/phileas/projects/indi_experiments/star_plotter/data/test30_1.fits", IMREAD_UNCHANGED);
    // Check for failure
    if (image.empty()) 
    {
        cout << "Could not open or find the image" << endl;
        //cin.get(); //wait for any key press
        return -1;
    }
    String windowName = "FIT viewer"; //Name of the window
    namedWindow(windowName); // Create a window
    imshow(windowName, image); // Show our image inside the created window.
    waitKey(0); // Wait for any keystroke in the window
    destroyWindow(windowName); //destroy the created window
    return 0;
}*/