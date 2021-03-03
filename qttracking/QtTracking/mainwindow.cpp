#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <fitsio.h>
#include <iostream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPainter>

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

bool MainWindow::openFit(QString filename, QImage* image) {
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

    int width = size_x;
    int height = size_y;
    *image = QPixmap(width, height).toImage();

    for(int x(0); x < image->width(); x++) {
        for(int y (0); y < image->height(); y++) {
            image->setPixelColor(x, y, QColor(rarray[y*size_x + x], garray[y*size_x + x], barray[y*size_x + x]));
        }
    }

    free(rarray);
    free(garray);
    free(barray);
    fits_close_file(fptr, &status);
    return true;
}

void MainWindow::stretchImage(float intensity) {
    QImage tmp = imageLabel->pixmap()->toImage();

    unsigned long long int red_sum = 0;
    for(int x(0); x < image.width(); x++) {
        for(int y (0); y < image.height(); y++) {
            red_sum += tmp.pixelColor(x, y).red();
        }
    }
    float red_avg = (float)red_sum / (float)(image.width()*image.height());

    for(int x(0); x < image.width(); x++) {
        for(int y (0); y < image.height(); y++) {
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

    QImage tobecompared;
    while (dialog.exec() == QDialog::Accepted && !openFit(dialog.selectedFiles().first(), &tobecompared)) {}

    // TODO: Live Stack image
    // TODO: detect features
    // TODO: Draw Debug
}

void MainWindow::drawDebug() {
    QImage tmp = imageLabel->pixmap()->toImage();
    QPainter qPainter(&tmp);
    QPen pen(Qt::red);
    qPainter.setBrush(Qt::NoBrush);
    pen.setWidth(20);
    qPainter.setPen(pen);
    //painter.drawPoint(5,5);
    qPainter.drawRect(100,100,200,200);
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

    while (dialog.exec() == QDialog::Accepted && !openFit(dialog.selectedFiles().first(), &image)) {}
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;
    scrollArea->setVisible(true);
    imageLabel->adjustSize();
    imageLabel->setScaledContents(true);
    scrollArea->setWidgetResizable(true); // Fit the image to window
    stretchImage(30);
    drawDebug();
}

MainWindow::~MainWindow()
{
    delete ui;
}

