#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fitsio.h>
#include <iostream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPainter>

#define PIXVAL_THRESHOLD 10 // number of time the average pixel value
#define MIN_DISTANCE_BETWEEN_STARS 10
#define MAX_STEPS 100 // Max "Size" of a star (in pixel)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , imageLabel(new QLabel)
    , scrollArea(new QScrollArea)
{

    ui->setupUi(this);

    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    ///M66_Light_60_secs_2021-02-22T01-36-58_002.fits
    ///M66_Light_60_secs_2021-02-22T01-38-10_003.fits
    //openFit("/home/phileas/Pictures/M66_soir_4/Light/M66_Light_60_secs_2021-02-22T01-36-58_002.fits");
    //resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
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

bool MainWindow::openFit(QString filename, RawImage* rawimg) {
    fitsfile *fptr;
    char card[FLEN_CARD];
    int status = 0, nkeys, ii;

    // Open the file
    auto begin = std::chrono::high_resolution_clock::now();
    fits_open_file(&fptr, filename.toStdString().c_str(), READONLY, &status);
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

    rawimg->width = size_x;
    rawimg->height = size_y;
    if (rawimg->red != NULL)
        free(rawimg->red);
    if (rawimg->green != NULL)
        free(rawimg->green);
    if (rawimg->blue != NULL)
        free(rawimg->blue);

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
    if (rawimg->bw != NULL)
        free(rawimg->bw);
    rawimg->bw = (uint8_t*)malloc(rawimg->width*rawimg->height);

    for(size_t x(0); x < rawimg->width; x++) {
        for(size_t y (0); y < rawimg->height; y++) {
            rawimg->bw[y*rawimg->width + x] = rawimg->red[y*rawimg->width + x] +
                2*rawimg->green[y*rawimg->width + x] +
                rawimg->blue[y*rawimg->width + x];
        }
    }
}

void MainWindow::stretchImage(float intensity) {
    QImage tmp = imageLabel->pixmap()->toImage();

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
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
}

void MainWindow::createActions() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &MainWindow::callback_openFile);
    openAct->setShortcut(QKeySequence::Open);

    QAction *compareAct = fileMenu->addAction(tr("&Compare with..."), this, &MainWindow::callback_openFile_compare);
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

    std::cout<<"Star x:"<<result.x<<" y:"<<result.y<<" w"<<result.w<<" h"<<result.h<<std::endl;

    return result;
}

void listStars(std::vector<Rectf>* out_boxes, std::vector<Point2f>* out_centers,
               RawImage &grayimg, float threshold)
{
    uint8_t thrld = (uint8_t) threshold;
    std::cout << "Star threshold is "<<(unsigned int)thrld<<std::endl;
    size_t single_pixel_star = 0;
    for(size_t x(0); x < grayimg.width; x++) {
        for(size_t y (0); y < grayimg.height; y++) {
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

std::vector<Feature> matchStarsBruteForce(std::vector<Point2f>* point1, std::vector<Point2f>* pointref) {
    std::vector<Feature> movement_vector;
    for (std::vector<Point2f>::iterator it = point1->begin(); it != point1->end(); it++) {
        float smallest_distance = 10000000;
        Point2f* best_match;
        for (std::vector<Point2f>::iterator it2 = pointref->begin(); it2 != pointref->end(); it2++) {
            float distance = (it->x - it2->x) * (it->x - it2->x) + (it->y - it2->y) * (it->y - it2->y);
            if (distance < smallest_distance) {
                best_match = (Point2f*)&(*it2);
                smallest_distance = distance;
            }
        }
        if (smallest_distance < 10000000) {
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

void MainWindow::callback_openFile_compare() {
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

    while (dialog.exec() == QDialog::Accepted && !openFit(dialog.selectedFiles().first(), &image_b)) {}

    stackImageWithNewImage();

    // Detect features, Measure star's movement between images
    std::vector<Point2f> points1, points2;
    float avg1 = getImgAveragePixel(&image_a);
    std::cout << "Average IMGA is "<<avg1<<std::endl;

    float avg2 = getImgAveragePixel(&image_b);
    std::cout << "Average IMGB is "<<avg2<<std::endl;

    // Apply threshold and detect star's boundary when threshold is reached
    listStars(NULL, &points1, image_a, PIXVAL_THRESHOLD*avg1);

    listStars(NULL, &points2, image_b, PIXVAL_THRESHOLD*avg2);

    mFeatures = matchStarsBruteForce(&points1, &points2);

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
        FilteredFeatures.push_back(*it);
        sum_x += it->vector.x;
        sum_y += it->vector.y;
    }

    avg_x = (float)sum_x / (float)FilteredFeatures.size();
    avg_y = (float)sum_y / (float)FilteredFeatures.size();
    std::cout<<"Filtered Average x:"<<avg_x<<" y:"<<avg_y<<std::endl;

    std::cout<<FilteredFeatures.size()<<"/" << mFeatures.size() << "vectors kept after filtering"<<std::endl;
    mFeatures = FilteredFeatures;

    drawDebug();
}

void MainWindow::drawDebug() {
    QImage tmp = imageLabel->pixmap()->toImage();
    QPainter qPainter(&tmp);
    QPen pen(Qt::red);
    qPainter.setBrush(Qt::NoBrush);
    pen.setWidth(10);
    qPainter.setPen(pen);
    //painter.drawPoint(5,5);
    //qPainter.drawRect(100,100,200,200);
    for (std::vector<Feature>::iterator it = mFeatures.begin(); it != mFeatures.end(); it++) {
        it->display(&qPainter);
    }
    qPainter.end();
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
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

    while (dialog.exec() == QDialog::Accepted && !openFit(dialog.selectedFiles().first(), &image_a)) {}
    QImage tmp;
    RawToQImage(&image_a, &tmp);
    imageLabel->setPixmap(QPixmap::fromImage(tmp));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    imageLabel->adjustSize();
    imageLabel->setScaledContents(true);
    scrollArea->setWidgetResizable(true); // Fit the image to window
    stretchImage(30);
    //drawDebug();
}

void MainWindow::stackImageWithNewImage() {
    QImage tmp = imageLabel->pixmap()->toImage();

    if (tmp.width() != image_b.width || tmp.height() != image_b.height) {
        std::cout<<"Cannot stack images, they are not the same size"<<std::endl;
        return;
    }

    QImage tmp2;
    RawToQImage(&image_b, &tmp2);

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

    imageLabel->setPixmap(QPixmap::fromImage(tmp));
}

MainWindow::~MainWindow()
{
    delete ui;
}

