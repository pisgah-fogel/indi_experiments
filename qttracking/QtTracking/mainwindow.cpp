#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fitsio.h>
#include <iostream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPainter>
#include <QTimer>
#include <QFileInfo>
#include <QDateTime>
#include <QChartView>
#include <QLineSeries>
#include <QPolarChart>
#include <QValueAxis>
#include <QListWidget>

#define PIXVAL_THRESHOLD 10 // number of time the average pixel value
#define MIN_DISTANCE_BETWEEN_STARS 5
#define MAX_STEPS 255 // Max "Size" of a star (in pixel)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , imageLabel(new LabelImage(&image_a))
    , scrollArea(new QScrollArea)
{

    ui->setupUi(this);

    ui->zoomedcompare->setBackgroundRole(QPalette::Base);
    ui->zoomedcompare->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->zoomedcompare->setScaledContents(true);

    ui->zoomed->setBackgroundRole(QPalette::Base);
    ui->zoomed->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->zoomed->setScaledContents(true);

    ui->zoomedRef->setBackgroundRole(QPalette::Base);
    ui->zoomedRef->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->zoomedRef->setScaledContents(true);

    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    imageLabel->setZoomedLabel(ui->zoomed);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);

    ui->mainImageLayout->addWidget(scrollArea);
    //setCentralWidget(scrollArea);

    createActions();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(scanDirectory()));

    createGraph();

    createPolarChart();
}

/*
 * Example:
 * openFitRect("/home/phileas/Pictures/orion_soir_9/Light/Light_30_secs_2021-03-06T21-46-15_001.fits", &image_b, QRect(787, 155, 128, 128), 4);
 * DisplayRawImage_zoomedcompare(&image_b);
 */
void MainWindow::DisplayRawImage_zoomedcompare(RawImage *ptr) {
    if (ptr->width != size_t(imageLabel->getSelectionRect().width()) || ptr->height != size_t(imageLabel->getSelectionRect().height())) {
        std::cout<<"Error: MainWindow::DisplayRawImage_zoomedcompare: rawimage's width or height does not match the selection"<<std::endl;
    }
    QImage tmpimage;
    RawToQImageBW(ptr, &tmpimage); // Black and White
    ui->zoomedcompare->setPixmap(QPixmap::fromImage(tmpimage));
}

void MainWindow::DisplayRawImage_zoomed(RawImage *ptr) {
    if (ptr->width != size_t(imageLabel->getSelectionRect().width()) || ptr->height != size_t(imageLabel->getSelectionRect().height())) {
        std::cout<<"Error: MainWindow::DisplayRawImage_zoomedcompare: rawimage's width or height does not match the selection"<<std::endl;
    }
    QImage tmpimage;
    RawToQImageBW(ptr, &tmpimage); // Black and White
    ui->zoomed->setPixmap(QPixmap::fromImage(tmpimage));
}

// Meant to display a full FIT in the main imageLabel
void MainWindow::DisplayRawImage_imageLabel(RawImage *ptr) {
    imageLabel->fromRaw(ptr);
    scrollArea->setVisible(true);
    scrollArea->setWidgetResizable(true); // Fit the image to window
    //MainWindow::redGreenStackingChangeImage(&tmp);
    //MainWindow::stretchImage(imageLabel, 30);
}

void MainWindow::DisplayRawImage_zoomedRef(RawImage *ptr) {
    if (ptr->width != size_t(imageLabel->getSelectionRect().width()) || ptr->height != size_t(imageLabel->getSelectionRect().height())) {
        std::cout<<"Error: MainWindow::DisplayRawImage_zoomedcompare: rawimage's width or height does not match the selection"<<std::endl;
    }
    QImage tmpimage;
    RawToQImageBW(ptr, &tmpimage); // Black and White
    ui->zoomedRef->setPixmap(QPixmap::fromImage(tmpimage));
}

void MainWindow::RawToQImage(RawImage* raw, QImage* qimage) {
    if (raw->red == NULL || raw->blue == NULL || raw->green == NULL) {
        std::cout<<"Error: MainWindow::RawToQImage: Raw image is empty"<<std::endl;
        return;
    }
    *qimage = QPixmap(raw->width, raw->height).toImage();

    for(size_t x(0); x < raw->width; x++) {
        for(size_t y (0); y < raw->height; y++) {
            qimage->setPixelColor(x, y, QColor(raw->red[y*raw->width + x], raw->green[y*raw->width + x], raw->blue[y*raw->width + x]));
        }
    }
}

void MainWindow::RawToQImageBW(RawImage* raw, QImage* qimage) {
    if (raw->red == NULL || raw->blue == NULL || raw->green == NULL) {
        std::cout<<"Error: MainWindow::RawToQImageBW: Raw image is empty"<<std::endl;
        return;
    }
    *qimage = QPixmap(raw->width, raw->height).toImage();

    for(size_t x(0); x < raw->width; x++) {
        for(size_t y (0); y < raw->height; y++) {
            qimage->setPixelColor(x, y, QColor(raw->bw[y*raw->width + x], raw->bw[y*raw->width + x], raw->bw[y*raw->width + x]));
        }
    }
}

void MainWindow::RawToQImageRect(RawImage* raw, QImage* qimage, QRect rect) {
    if (raw->red == NULL || raw->blue == NULL || raw->green == NULL) {
        std::cout<<"Error: MainWindow::RawToQImage: Raw image is empty"<<std::endl;
        return;
    }
    *qimage = QPixmap(rect.width(), rect.height()).toImage();

    for(size_t x(rect.x()), x2(0); x < size_t(rect.x()+rect.width()); x++, x2++) {
        for(size_t y (rect.y()), y2(0); y < size_t(rect.y()+rect.height()); y++, y2++) {
            qimage->setPixelColor(x2, y2, QColor(raw->red[y*raw->width + x], raw->green[y*raw->width + x], raw->blue[y*raw->width + x]));
        }
    }
}

bool MainWindow::RawToQImageRectBW(RawImage* raw, QImage* qimage, QRect rect) {
    if (raw->red == NULL || raw->blue == NULL || raw->green == NULL) {
        std::cout<<"Error: MainWindow::RawToQImageRectBW: Raw image is empty"<<std::endl;
        return false;
    }
    if (size_t(rect.x()+rect.width()) > raw->width || size_t(rect.y()+rect.height()) > raw->height) {
        std::cout<<"Error: MainWindow::RawToQImageRectBW: Raw image is too small for this selection"<<std::endl;
        return false;
    }

    *qimage = QPixmap(rect.width(), rect.height()).toImage();

    for(size_t x(rect.x()), x2(0); x < size_t(rect.x()+rect.width()); x++, x2++) {
        for(size_t y (rect.y()), y2(0); y < size_t(rect.y()+rect.height()); y++, y2++) {
            qimage->setPixelColor(x2, y2, QColor(raw->bw[y*raw->width + x], raw->bw[y*raw->width + x], raw->bw[y*raw->width + x]));
        }
    }
    return true;
}

bool MainWindow::openFitRect(QString filename, RawImage* rawimg, QRect rect, int binding=4) {
    std::cout<<"openFitRect x:"<<rect.x()<<" y:"<<rect.y()<<" w:"<<rect.width()<<" h:"<<rect.height()<<" binding:"<<binding<<std::endl;
    fitsfile *fptr;
    int status = 0, nkeys;

    if (rect.x() < 0 || rect.y() < 0) {
        // TODO: display in GUI
        std::cout<<"Error: Please select a valid star region"<<std::endl;
        return false;
    }

    // Open the file
    auto begin = std::chrono::high_resolution_clock::now();
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists() || !fileinfo.isFile() || !fileinfo.isReadable()) {
        std::cout<<"Error: Cannot read file"<<std::endl;
        return false;
    }

    fits_open_file(&fptr, fileinfo.absoluteFilePath().toStdString().c_str(), READONLY, &status);
    fits_get_hdrspace(fptr, &nkeys, NULL, &status);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Open file: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << "ns" << std::endl;

    // Read every pixel, stretch resize and display
    int bitpix;
    int ret = fits_get_img_type(fptr, &bitpix, &status);
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

    long naxes[3];
    ret = fits_get_img_size(fptr, naxis /*maxdim*/, naxes, &status);
    if (ret != 0) {
        std::cout << "Error: could not determe how many channels are in the image" << std::endl;
        fits_close_file(fptr, &status);
        return false;
    }

    int size_x = naxes[0], size_y = naxes[1];

    if (naxes[2] != 3) {
        std::cout<<"Error: only 3 channels images are supported, this one is "<<naxes<<std::endl;
        fits_close_file(fptr, &status);
        return false;
    }

    int result_size_x = rect.width();
    int result_size_y = rect.height();
    // TODO: check required array size
    std::cout<<"Each channel takes (Crop) "<<(result_size_x)*(result_size_y)+1<<"o"<<std::endl;
    unsigned char *rarray = (unsigned char *)malloc((result_size_x)*(result_size_y)+1); // Red channel
    unsigned char *garray = (unsigned char *)malloc((result_size_x)*(result_size_y)+1); // Green channel
    unsigned char *barray = (unsigned char *)malloc((result_size_x)*(result_size_y)+1); // Blue channel

    long pixel_min_x = 1 + rect.x()*binding;
    long pixel_min_y = 1 + rect.y()*binding;
    long pixel_max_x = pixel_min_x+result_size_x*binding-1;
    long pixel_max_y = pixel_min_y+result_size_y*binding-1;

    std::cout<<"FITS Window: x:"<<pixel_min_x<<" y:"<<pixel_min_y<<std::endl;
    std::cout<<"             x:"<<pixel_max_x<<" y:"<<pixel_max_y<<std::endl;

    long fpixel [] = {pixel_min_x, pixel_min_y, 1, 1 /*First dimension to be read*/};
    int anynul;

    long lpixel [] = {pixel_max_x, pixel_max_y, 1, 2 /*Last dimension to be read*/};
    long inc [] = {binding, binding, 1};
    ret = ffgsv (fptr, TBYTE, fpixel, lpixel, inc,
           NULL, rarray, &anynul, &status);

    fpixel[2] = 2;
    lpixel[2] = 2;
    ret = ffgsv (fptr, TBYTE, fpixel, lpixel, inc,
           NULL, garray, &anynul, &status);

    fpixel[2] = 3;
    lpixel[2] = 3;

    ret = ffgsv (fptr, TBYTE, fpixel, lpixel, inc,
           NULL, barray, &anynul, &status);

    if (status)
    fits_report_error(stderr, status);

    rawimg->width = result_size_x;
    rawimg->height = result_size_y;
    if (rawimg->red != NULL) {
        free(rawimg->red);
        rawimg->red = NULL;
    }
    if (rawimg->green != NULL) {
        free(rawimg->green);
        rawimg->green = NULL;
    }
    if (rawimg->blue != NULL) {
        free(rawimg->blue);
        rawimg->blue = NULL;
    }

    rawimg->red = rarray;
    rawimg->green = garray;
    rawimg->blue = barray;
    computeBWfromRawImage(rawimg);

    fits_close_file(fptr, &status);
    return true;
}

bool MainWindow::openFit(QString filename, RawImage* rawimg, int binding=4) {
    fitsfile *fptr;
    //char card[FLEN_CARD];
    int status = 0, nkeys, ii;

    // Open the file
    auto begin = std::chrono::high_resolution_clock::now();
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists() || !fileinfo.isFile() || !fileinfo.isReadable()) {
        std::cout<<"Error: Cannot read file"<<std::endl;
        return false;
    }

    fits_open_file(&fptr, fileinfo.absoluteFilePath().toStdString().c_str(), READONLY, &status);
    fits_get_hdrspace(fptr, &nkeys, NULL, &status);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Open file: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << "ns" << std::endl;

    // List every meta data
    /*
    begin = std::chrono::high_resolution_clock::now();
    for (ii = 1; ii <= nkeys; ii++)
    {
        fits_read_record(fptr, ii, card, &status);
        printf("%s\n", card);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Read META: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count() << "ns" << std::endl;
    */

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

    std::cout<<"Each channel takes (full image) "<<(size_x/binding)*(size_y/binding)<<"o"<<std::endl;
    unsigned char *rarray = (unsigned char *)malloc((size_x/binding)*(size_y/binding)); // Red channel
    unsigned char *garray = (unsigned char *)malloc((size_x/binding)*(size_y/binding)); // Green channel
    unsigned char *barray = (unsigned char *)malloc((size_x/binding)*(size_y/binding)); // Blue channel
    long fpixel [] = {1, 1, 1, 1}; // Size: NAXIS
    // fpixel[0] goes from 1 to NAXIS1
    // fpixel[1] goes from 1 to NAXIS2
    int anynul;

    long lpixel [] = {size_x, size_y, 1, 2}; // TODO: Size before binding ?
    long inc [] = {binding, binding, 1, 1};
    ret = ffgsv (fptr, TBYTE, fpixel, lpixel, inc,
           NULL, rarray, &anynul, &status);
/*
    ret = ffgpxv(fptr, TBYTE, fpixel,
                size_x*size_y, NULL, rarray,
                &anynul, &status);*/
    fpixel[2] = 2;
    lpixel[2] = 2;
    ret = ffgsv (fptr, TBYTE, fpixel, lpixel, inc,
           NULL, garray, &anynul, &status);
    /*ret = ffgpxv(fptr, TBYTE, fpixel,
                size_x*size_y, NULL, garray,
                &anynul, &status);*/
    fpixel[2] = 3;
    lpixel[2] = 3;
    /*ret = ffgpxv(fptr, TBYTE, fpixel,
                size_x*size_y, NULL, barray,
                &anynul, &status);*/
    ret = ffgsv (fptr, TBYTE, fpixel, lpixel, inc,
           NULL, barray, &anynul, &status);

    if (status)
    fits_report_error(stderr, status);

    rawimg->width = size_x/binding;
    rawimg->height = size_y/binding;
    if (rawimg->red != NULL) {
        free(rawimg->red);
        rawimg->red = NULL;
    }
    if (rawimg->green != NULL) {
        free(rawimg->green);
        rawimg->green = NULL;
    }
    if (rawimg->blue != NULL) {
        free(rawimg->blue);
        rawimg->blue = NULL;
    }

    rawimg->red = rarray;
    rawimg->green = garray;
    rawimg->blue = barray;
    computeBWfromRawImage(rawimg);

    // rarray, garray and barray will be free by ~RawImage or
    // next time we open a new image

    fits_close_file(fptr, &status);
    return true;
}

void MainWindow::computeBWfromRawImage(RawImage* rawimg) {
    if (rawimg->bw != NULL) {
        free(rawimg->bw);
        rawimg->bw = NULL;
    }

    std::cout<<"BW channel takes "<<size_t(rawimg->width*rawimg->height+1)<<"o"<<std::endl;
    rawimg->bw = (uint8_t*)malloc(size_t(rawimg->width*rawimg->height+1));

    for(size_t x(0); x < rawimg->width; x++) {
        for(size_t y (0); y < rawimg->height; y++) {
            unsigned int val = rawimg->red[y*rawimg->width + x] +
                    2*rawimg->green[y*rawimg->width + x] +
                    rawimg->blue[y*rawimg->width + x];
            if (val > 255)
                val = 255;
            rawimg->bw[y*rawimg->width + x] = (unsigned char)val;
        }
    }
}

void MainWindow::stretchImage(QLabel* label, float intensity) {
    QImage tmp = label->pixmap()->toImage();

    unsigned long long int red_sum = 0;
    for(int x(0); x < tmp.width(); x++) {
        for(int y (0); y < tmp.height(); y++) {
            red_sum += tmp.pixelColor(x, y).red();
        }
    }
    float red_avg = (float)red_sum / (float)(tmp.width()*tmp.height());

    for(int x(0); x < tmp.width(); x++) {
        for(int y (0); y < tmp.height(); y++) {
            QColor pix = tmp.pixelColor(x, y);
            // TODO: find a better way to do it
            int r = (int)((float)pix.red())*(intensity / red_avg);
            if (r > 255) r=255;
            if (r < 0) r=0;
            int g = (int)((float)pix.green())*(intensity / red_avg);
            if (g > 255) g=255;
            if (g < 0) g=0;
            int b = (int)((float)pix.blue())*(intensity / red_avg);
            if (b > 255) b=255;
            if (b < 0) b=0;
            tmp.setPixelColor(x, y, QColor(r, g, b));
        }
    }
    label->setPixmap(QPixmap::fromImage(tmp));
}

void MainWindow::scanDirectory() {
    if (!trackingDirectory)
        return;

    // Looking into imageDirectory
    QDir directory(imageDirectory);
    QStringList images = directory.entryList(QStringList() << "*.fits" << "*.FITS",QDir::Files);
    QString filetoprocess;
    bool somethingtodo = false;
    foreach(QString filename, images) {
        QString img_filename = imageDirectory+ "/" +filename;
        if (old_files.find(img_filename) == old_files.end()) {
            std::cout<<"New file "<<img_filename.toStdString();
            QFileInfo fileInfo(img_filename);
            QDateTime t = fileInfo.lastModified();
            std::cout<<" - "<<t.toString().toStdString()<<std::endl;
            old_files[img_filename] = t;
            ui->listWidget->addItem(img_filename);
            somethingtodo = true;
            filetoprocess = img_filename;
            break; // TODO: handle many files
        }
    }

    if (somethingtodo) {
        // Do something with filetoprocess
        if (image_a.empty()) {
            // Considere the image as our reference
            // TODO
            openFit(filetoprocess, &image_a);
            DisplayRawImage_imageLabel(&image_a);
        } else {
            if (!image_b.empty()) {
             // move image_b to image_a and do as if image_b was empty
             image_a = image_b;
             image_b = RawImage();
             //DisplayRawImage_zoomed(&image_a);
             DisplayRawImage_zoomedRef(&image_a);
            }

            //load only a subset of image_b in order to compare it with image_a
            openFitRect(filetoprocess, &image_b, imageLabel->getSelectionRect());
            DisplayRawImage_zoomedcompare(&image_b);

            measureVectorBtwImagesBox();
            drawDebug();
        }
    }

    timer->start(3000); // TODO: Add variable delay
}

void MainWindow::callback_watch_directory() {
    QFileDialog dialog(this, tr("Track Directory"));
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    if (dialog.exec() == QDialog::Accepted) {
        trackingDirectory = true;
        imageDirectory = dialog.selectedFiles().first();
        timer->start(3000); // TODO: Add variable delay
    }
}

void MainWindow::createActions() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &MainWindow::callback_openFile);
    openAct->setShortcut(QKeySequence::Open);

    QAction *compareAct = fileMenu->addAction(tr("&Compare with..."), this, &MainWindow::callback_openFile_compare);

    QAction *watchAct = fileMenu->addAction(tr("&Watch directory..."), this, &MainWindow::callback_watch_directory);
}

float getImgAveragePixel(RawImage* rawimg) {
    unsigned long long int sum = 0;
    for(size_t x(0); x < rawimg->width; x++) {
        for(size_t y (0); y < rawimg->height; y++) {
            sum += rawimg->bw[y*rawimg->width + x];
        }
    }
    // TODO: store average value so I do not calculate it each time ?
    return (float)sum/(float)(rawimg->width*rawimg->height);
}

float getImgAveragePixelRect(RawImage* rawimg, QRect rect) {
    unsigned long long int sum = 0;
    for(size_t x(rect.x()); x < (size_t)(rect.x() + rect.width()); x++) {
        for(size_t y (rect.y()); y < (size_t)(rect.y() + rect.height()); y++) {
            sum += rawimg->bw[y*rawimg->width + x];
        }
    }
    // TODO: store average value so I do not calculate it each time ?
    return (float)sum/(float)(rect.width()*rect.height());
}

Rectf getStarBoundary(RawImage &rawimg, int x, int y, unsigned int thrld) {
    Rectf result;

    // TODO: check tmpx < width >= 0 (+ same for tmpy)
    // Find top (y small)
    int tmpx = x, tmpy = y;
    for (size_t steps = 0; steps < MAX_STEPS; steps ++) {
        if (tmpx < 1)
            break;
        else if ((size_t)tmpx + 1 >= rawimg.width)
            break;
        if (tmpy < 1)
            break;
        else if ((size_t)tmpy + 1 >= rawimg.height)
            break;
        if (rawimg.bw[(tmpy-1)*rawimg.width + tmpx] > thrld) {
            tmpy -= 1;
        }
        else if (rawimg.bw[(tmpy-1)*rawimg.width + tmpx-1] > thrld) {
            tmpy -= 1;
            tmpx -= 1;
        }
        else if (rawimg.bw[(tmpy-1)*rawimg.width + tmpx+1] > thrld) {
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
        if (tmpx < 1)
            break;
        else if ((size_t)tmpx + 1 >= rawimg.width)
            break;
        if (tmpy < 1)
            break;
        else if ((size_t)tmpy + 1 >= rawimg.height)
            break;
        if (rawimg.bw[(tmpy+1)*rawimg.width + tmpx] > thrld) {
            tmpy += 1;
        }
        else if (rawimg.bw[(tmpy+1)*rawimg.width + tmpx-1] > thrld) {
            tmpy += 1;
            tmpx -= 1;
        }
        else if (rawimg.bw[(tmpy+1)*rawimg.width + tmpx+1] > thrld) {
            tmpy += 1;
            tmpx += 1;
        }
        else
            break;
    }
    result.h = tmpy - result.y;

    tmpx = x;
    tmpy = y;
    // Find left (x small)
    for (size_t steps = 0; steps < MAX_STEPS; steps ++) {
        if (tmpx < 1)
            break;
        else if ((size_t)tmpx + 1 >= rawimg.width)
            break;
        if (tmpy < 1)
            break;
        else if ((size_t)tmpy + 1 >= rawimg.height)
            break;

        if (rawimg.bw[(tmpy)*rawimg.width + tmpx-1] > thrld) {
            tmpx -= 1;
        }
        else if (rawimg.bw[(tmpy-1)*rawimg.width + tmpx-1] > thrld) {
            tmpx -= 1;
            tmpy -= 1;
        }
        else if (rawimg.bw[(tmpy+1)*rawimg.width + tmpx-1] > thrld) {
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
        if (tmpx < 1)
            break;
        else if ((size_t)tmpx + 1 >= rawimg.width)
            break;
        if (tmpy < 1)
            break;
        else if ((size_t)tmpy + 1 >= rawimg.height)
            break;

        if (rawimg.bw[(tmpy)*rawimg.width + tmpx+1] > thrld) {
            tmpx += 1;
        }
        else if (rawimg.bw[(tmpy-1)*rawimg.width + tmpx+1] > thrld) {
            tmpx += 1;
            tmpy -= 1;
        }
        else if (rawimg.bw[(tmpy+1)*rawimg.width + tmpx+1] > thrld) {
            tmpx += 1;
            tmpy += 1;
        }
        else
            break;
    }
    result.w = tmpx - result.x;

    //std::cout<<"Star x:"<<result.x<<" y:"<<result.y<<" w"<<result.w<<" h"<<result.h<<std::endl;

    return result;
}

void MainWindow::listStars(std::vector<Rectf>* out_boxes, std::vector<Point2f>* out_centers,
               RawImage &grayimg, float threshold)
{
    uint8_t thrld = (uint8_t) threshold;
    std::cout << "Star threshold is "<<(unsigned int)thrld<<std::endl;
    size_t single_pixel_star = 0;
    for(size_t x(0); x < grayimg.width; x++) {
        for(size_t y (0); y < grayimg.height; y++) {
            if (grayimg.bw[y*grayimg.width + x] > thrld) {
                if (ui->zoomedRef->pixmap() != NULL) {
                    QImage tmp = ui->zoomedRef->pixmap()->toImage();
                    QPainter qPainter(&tmp);
                    QPen pen(Qt::magenta);
                    qPainter.setBrush(Qt::NoBrush);
                    pen.setWidth(1);
                    qPainter.setPen(pen);
                    qPainter.drawPoint(x,y);
                    qPainter.end();
                    ui->zoomedRef->setPixmap(QPixmap::fromImage(tmp));
                }
            }
        }
    }
    for(size_t x(0); x < grayimg.width; x++) {
        for(size_t y (0); y < grayimg.height; y++) {
            if (grayimg.bw[y*grayimg.width + x] > thrld) {
                Rectf tmp = getStarBoundary(grayimg, x, y, thrld);

                if (tmp.w <= 1 && tmp.h <= 1) {
                    single_pixel_star ++;
                    continue;
                }

                Point2f center = Point2f(tmp.x + tmp.w/2, tmp.y + tmp.h/2);

                x += tmp.w + MIN_DISTANCE_BETWEEN_STARS;
                y += tmp.h + MIN_DISTANCE_BETWEEN_STARS;

                if (out_boxes != NULL)
                    out_boxes->push_back(tmp);
                if (out_centers != NULL)
                    out_centers->push_back(center);
            }
        }
    }

    if (out_boxes != NULL)
    std::cout<<out_boxes->size()<<" stars detected"<<std::endl;
    else if (out_centers != NULL)
    std::cout<<out_centers->size()<<" stars detected"<<std::endl;

    std::cout<<"Skipped single pixel star "<<single_pixel_star<<std::endl;
}

void MainWindow::findStarInRect(std::vector<Rectf>* out_boxes, std::vector<Point2f>* out_centers,
                    RawImage &grayimg, float threshold, QRect box) {
    if (box.x() < 0 || box.y() <0 || box.x()+box.width()>grayimg.width || box.y()+box.height()>grayimg.height) {
        std::cout<<"Warning: findStarInRect: Box position is Invalid"<<std::endl;
        listStars(out_boxes, out_centers, grayimg, threshold);
        return;
    }

    uint8_t thrld = (uint8_t) threshold;
    std::cout << "Star threshold is "<<(unsigned int)thrld<<std::endl;
    size_t single_pixel_star = 0;
    for(size_t x(box.x()); x < (size_t)(box.x()+box.width()); x++) {
        for(size_t y (box.y()); y < (size_t)(box.y()+box.height()); y++) {
            if (grayimg.bw[y*grayimg.width + x] > thrld) {
                Rectf tmp = getStarBoundary(grayimg, x, y, thrld);

                if (tmp.w == 0 && tmp.h == 0) {
                    single_pixel_star ++;
                    continue;
                }

                Point2f center = Point2f(tmp.x + tmp.w/2, tmp.y + tmp.h/2);

                x += tmp.w + MIN_DISTANCE_BETWEEN_STARS;
                y += tmp.h + MIN_DISTANCE_BETWEEN_STARS;

                if (out_boxes != NULL)
                    out_boxes->push_back(tmp);
                if (out_centers != NULL)
                    out_centers->push_back(center);
            }
        }
    }

    if (out_boxes != NULL)
    std::cout<<out_boxes->size()<<" stars detected"<<std::endl;
    else if (out_centers != NULL)
    std::cout<<out_centers->size()<<" stars detected"<<std::endl;

    std::cout<<"Skipped single pixel star "<<single_pixel_star<<std::endl;
}

std::vector<Feature> matchStarsBruteForce(std::vector<Point2f>* point1, std::vector<Point2f>* pointref, float maxdistance) {
    std::vector<Feature> movement_vector;
    for (std::vector<Point2f>::iterator it = point1->begin(); it != point1->end(); it++) {
        float smallest_distance = maxdistance;
        Point2f* best_match;
        for (std::vector<Point2f>::iterator it2 = pointref->begin(); it2 != pointref->end(); it2++) {
            float distance = (it->x - it2->x) * (it->x - it2->x) + (it->y - it2->y) * (it->y - it2->y);
            if (distance < smallest_distance) {
                best_match = (Point2f*)&(*it2);
                smallest_distance = distance;
            }
        }
        if (smallest_distance < maxdistance) {
            Feature tmp;
            tmp.origin = *it;
            tmp.destination = *best_match;
            tmp.vector = Point2f(best_match->x - it->x, best_match->y - it->y);
            movement_vector.push_back(tmp);
        }
    }

    std::cout<<"Failed to match: "<<point1->size() - movement_vector.size()<< "stars" <<std::endl;

    return movement_vector;
}

void MainWindow::measureVectorBtwImages() {
    // Detect features, Measure star's movement between images
    std::vector<Point2f> points1, points2;
    float avg1 = getImgAveragePixel(&image_a);
    std::cout << "Average IMGA is "<<avg1<<std::endl;

    float avg2 = getImgAveragePixel(&image_b);
    std::cout << "Average IMGB is "<<avg2<<std::endl;

    // Apply threshold and detect star's boundary when threshold is reached
    listStars(NULL, &points1, image_a, PIXVAL_THRESHOLD*avg1);

    listStars(NULL, &points2, image_b, PIXVAL_THRESHOLD*avg2);

    mFeatures = matchStarsBruteForce(&points1, &points2, 64);

    if (mFeatures.size() <= 0) {
        std::cout<<"No star matches"<<std::endl;
        return;
    }

    double sum_x = 0, sum_y = 0;
    for (std::vector<Feature>::iterator it = mFeatures.begin(); it != mFeatures.end(); it++) {
        sum_x += it->vector.x;
        sum_y += it->vector.y;
    }
    float avg_x = (float)sum_x / (float)mFeatures.size();
    float avg_y = (float)sum_y / (float)mFeatures.size();
    std::cout<<"Average before filtering x:"<<avg_x<<" y:"<<avg_y<<std::endl;

    // Filtering
    sum_x = 0, sum_y = 0;
    const float skip_threshold = 15.f;
    int skipped = 0;
    std::vector<Feature> FilteredFeatures;
    for (std::vector<Feature>::iterator it = mFeatures.begin(); it != mFeatures.end(); it++) {
        //std::cout<<"Vector x:"<<it->vector.x<<" y:"<<it->vector.y<<std::endl;
        if (abs(it->vector.x - avg_x) > skip_threshold) {
            //std::cout<<"x too high/low"<<std::endl;
            skipped++;
            continue;
        }
        else if (abs(it->vector.y - avg_y) > skip_threshold) {
            //std::cout<<"y too high/low"<<std::endl;
            skipped++;
            continue;
        }
        FilteredFeatures.push_back(*it);
        sum_x += it->vector.x;
        sum_y += it->vector.y;
    }

    if (FilteredFeatures.size() <= 0) {
        std::cout<<"No star matches - after filtering"<<std::endl;
        return;
    }

    avg_x = (float)sum_x / (float)FilteredFeatures.size();
    avg_y = (float)sum_y / (float)FilteredFeatures.size();
    std::cout<<"Filtered Average x:"<<avg_x<<" y:"<<avg_y<<std::endl;

    std::cout<<FilteredFeatures.size()<<"/" << mFeatures.size() << "vectors kept after filtering"<<std::endl;
    mFeatures = FilteredFeatures;

    addPointToPolarChart(avg_x, avg_y);
    addValueToGraph(avg_x, avg_y);
}

void MainWindow::measureVectorBtwImagesBox() {
    QRect box = imageLabel->getSelectionRect();
    // Detect features, Measure star's movement between images
    std::vector<Point2f> points1, points2;

    float avg1;
    if (size_t(box.x() + box.width()) <= image_a.width) {
        avg1 = getImgAveragePixel(&image_a);
    } else {
        avg1 = getImgAveragePixelRect(&image_a, box);
    }
    std::cout << "Average IMGA is "<<avg1<<std::endl;

    float avg2;
    if (size_t(box.x() + box.width()) <= image_b.width) {
        avg2 = getImgAveragePixel(&image_b);
    } else {
        avg2 = getImgAveragePixelRect(&image_b, box);
    }
    std::cout << "Average IMGB is "<<avg2<<std::endl;

    // Apply threshold and detect star's boundary when threshold is reached
    if (size_t(box.x() + box.width()) <= image_a.width) {
        listStars(NULL, &points1, image_a, PIXVAL_THRESHOLD*avg1);
    } else {
        findStarInRect(NULL, &points1, image_a, PIXVAL_THRESHOLD*avg1, box);
    }

    if (size_t(box.x() + box.width()) <= image_b.width) {
        listStars(NULL, &points2, image_b, PIXVAL_THRESHOLD*avg2);
    } else {
        findStarInRect(NULL, &points2, image_b, PIXVAL_THRESHOLD*avg2, box);
    }

    mFeatures = matchStarsBruteForce(&points1, &points2, 64);

    std::cout<<"Matched stars "<<mFeatures.size()<<std::endl;

    if (mFeatures.size() <= 0) {
        std::cout<<"No star matches"<<std::endl;
        return;
    }

    double sum_x = 0, sum_y = 0;
    for (std::vector<Feature>::iterator it = mFeatures.begin(); it != mFeatures.end(); it++) {
        sum_x += it->vector.x;
        sum_y += it->vector.y;
    }
    float avg_x = (float)sum_x / (float)mFeatures.size();
    float avg_y = (float)sum_y / (float)mFeatures.size();
    std::cout<<"Average before filtering x:"<<avg_x<<" y:"<<avg_y<<std::endl;

    // Filtering
    /*
    sum_x = 0, sum_y = 0;
    const float skip_threshold = 15.f;
    int skipped = 0;
    std::vector<Feature> FilteredFeatures;
    for (std::vector<Feature>::iterator it = mFeatures.begin(); it != mFeatures.end(); it++) {
        //std::cout<<"Vector x:"<<it->vector.x<<" y:"<<it->vector.y<<std::endl;
        if (abs(it->vector.x - avg_x) > skip_threshold) {
            //std::cout<<"x too high/low"<<std::endl;
            skipped++;
            continue;
        }
        else if (abs(it->vector.y - avg_y) > skip_threshold) {
            //std::cout<<"y too high/low"<<std::endl;
            skipped++;
            continue;
        }
        FilteredFeatures.push_back(*it);
        sum_x += it->vector.x;
        sum_y += it->vector.y;
    }

    if (FilteredFeatures.size() <= 0) {
        std::cout<<"No star matches - after filtering"<<std::endl;
        return;
    }

    avg_x = (float)sum_x / (float)FilteredFeatures.size();
    avg_y = (float)sum_y / (float)FilteredFeatures.size();
    std::cout<<"Filtered Average x:"<<avg_x<<" y:"<<avg_y<<std::endl;

    std::cout<<FilteredFeatures.size()<<"/" << mFeatures.size() << "vectors kept after filtering"<<std::endl;
    mFeatures = FilteredFeatures;
    */

    addPointToPolarChart(avg_x, avg_y);
    addValueToGraph(avg_x, avg_y);

    qreal newx = box.x()-box.width()/2+points2.at(0).x;
    qreal newy = box.y()-box.height()/2+points2.at(0).y;
    std::cout<<"Old Box was x:"<<box.x()<<" y:"<<box.y()<<" w:"<<box.width()<<" h:"<<box.height()<<std::endl;
    std::cout<<"Star position: x:"<<points2.at(0).x<<" y:"<<points2.at(0).y<<std::endl;
    //imageLabel->moveRect(newx, newy);
    std::cout<<"New Box is x:"<<newx<<" y:"<<newy<<std::endl;
}

void MainWindow::callback_openFile_compare() {

    if (image_a.empty()) {
        // TODO: GUI error
        std::cout<<"Error: MainWindow::callback_openFile_compare: Please open a file before"<<std::endl;
        return;
    }

    QFileDialog dialog(this, tr("Open File"));
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    mimeTypeFilters.append("image/fits");
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/fits");
    if (QFileDialog::AcceptOpen == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("fits");

    if (dialog.exec() != QDialog::Accepted) {
        return; // Aborded by user
    }

    // TODO: QProgressDialog
    this->update();

    if (!image_b.empty()) {
        // move image_b to image_a and do as if image_b was empty
        image_a = image_b;
        image_b = RawImage();
        //DisplayRawImage_zoomed(&image_a);
        DisplayRawImage_zoomedRef(&image_a);
    }

    if (openFitRect(dialog.selectedFiles().first(), &image_b, imageLabel->getSelectionRect())) {
        //stackImageWithImage_b();
        measureVectorBtwImagesBox();
        //measureVectorBtwImages();
        // TODO: display small rect
        DisplayRawImage_zoomedcompare(&image_b);
        drawDebug();
    } else {
        std::cout<<"Error: MainWindow::callback_openFile_compare: Open FITS failed"<<std::endl;
    }
}

void MainWindow::drawDebug() {
    if (ui->zoomedRef->pixmap() == NULL)
        return;
    QImage tmp = ui->zoomedRef->pixmap()->toImage();
    QPainter qPainter(&tmp);
    QPen pen(Qt::red);
    qPainter.setBrush(Qt::NoBrush);
    pen.setWidth(1);
    qPainter.setPen(pen);
    //painter.drawPoint(5,5);
    //qPainter.drawRect(100,100,200,200);
    for (std::vector<Feature>::iterator it = mFeatures.begin(); it != mFeatures.end(); it++) {
        it->display(&qPainter);
    }
    qPainter.end();
    ui->zoomedRef->setPixmap(QPixmap::fromImage(tmp));
}

void MainWindow::callback_openFile() {
    QFileDialog dialog(this, tr("Open File"));
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    mimeTypeFilters.append("image/fits");
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/fits");
    if (QFileDialog::AcceptOpen == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("fits");

    if (dialog.exec() != QDialog::Accepted)
        return; // Aborded by user
    if (!openFit(dialog.selectedFiles().first(), &image_a)) {
        std::cout<<"Error: cannon open FITS file (reference)"<<std::endl;
        return;
    }

    // TODO: QProgressDialog
    DisplayRawImage_imageLabel(&image_a);
}

void MainWindow::stackImageWithImage_b() {
    //QImage tmp = imageLabel->pixmap()->toImage(); // Works but get quite messy with time + poor guiding
    QImage tmp;
    RawToQImage(&image_a, &tmp);

    if (tmp.width() != image_b.width || tmp.height() != image_b.height) {
        std::cout<<"Cannot stack images, they are not the same size"<<std::endl;
        return;
    }

    QImage tmp2;
    RawToQImage(&image_b, &tmp2);
    MainWindow::redGreenStackingChangeImage(&tmp2);

    for(int x(0); x < tmp.width(); x++) {
        for(int y (0); y < tmp.height(); y++) {
            QColor pix = tmp.pixelColor(x, y);
            QColor pix2 = tmp2.pixelColor(x, y);
            int r = pix.red()+pix2.red();
            if (r > 255)
                r = 255;
            else if (r<0)
                r = 0;
            int g = pix.green()+pix2.green();
            if (g > 255)
                g = 255;
            else if (g<0)
                g = 0;
            int b = pix.blue()+pix2.blue();
            if (b > 255)
                b = 255;
            else if (b<0)
                b = 0;
            // TODO: avoid overflow
            tmp.setPixelColor(x, y, QColor(r, g, b));
        }
    }

    //imageLabel->setPixmap(QPixmap::fromImage(tmp));
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
    imageLabel->adjustSize();
    imageLabel->setScaledContents(true);
    stretchImage(imageLabel, 30);
}

MainWindow::~MainWindow()
{
    delete ui;
}

