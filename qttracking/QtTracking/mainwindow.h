#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include <QPainter>
#include <QtCharts>

#include "qlabelimage.h"

class LabelImage;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Point2f {
public:
    Point2f() {
        x = 0.f;
        y = 0.f;
    }
    Point2f(float a, float b) {
        x = a;
        y = b;
    }
    float x;
    float y;
};

class Rectf {
public:
    Rectf() {
        x = 0.f;
        y = 0.f;
        w = 0.f;
        h = 0.f;
    }
    float x;
    float y;
    float w;
    float h;
};

class Feature {
    public:
    Point2f origin;
    Point2f destination;
    Point2f vector;
    void display(QPainter* painter) {
        painter->drawPoint(destination.x,destination.y);
        painter->drawRect(origin.x-10,origin.y-10, 20, 20);
        painter->drawLine(origin.x,origin.y, destination.x,destination.y);
    }
};

class RawImage {
public:
    bool empty() {
        return blue == NULL && red == NULL && green == NULL && bw == NULL;
    }
    RawImage() {
        width = 0;
        height = 0;
        blue = NULL;
        red = NULL;
        green = NULL;
        bw = NULL;
    }
    ~RawImage() {
        if (blue != NULL)
            free(blue);
        if (red != NULL)
            free(red);
        if (green != NULL)
            free(green);
        if (bw != NULL)
            free(bw);
    }
    size_t width, height;
    uint8_t* red;
    uint8_t* green;
    uint8_t* blue;
    uint8_t* bw;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createActions();
    void stretchImage(QLabel* label, float intensity);
    static void RawToQImageRect(RawImage* raw, QImage* qimage, QRect rect);
    static void RawToQImageRectBW(RawImage* raw, QImage* qimage, QRect rect);
private slots:
    void callback_openFile();
    void callback_openFile_compare();
    void callback_watch_directory();
    void drawDebug();
    void scanDirectory();
private:
    void stackImageWithImage_b();
    static bool openFit(QString filename, RawImage *image, int binding);
    static bool openFitRect(QString filename, RawImage* rawimg, QRect rect, int binding);
    static void RawToQImage(RawImage* raw, QImage* qimage);
    static void computeBWfromRawImage(RawImage* rawimg);
    void measureVectorBtwImages();

    void createPolarChart();
    void addPointToPolarChart(float x, float y);

    void createRedGreenStacking();
    void redGreenStackingChangeImage(QImage* image);

    void createGraph();
    void addValueToGraph(float vec_x, float vec_y);

    Ui::MainWindow *ui;
    LabelImage *imageLabel;
    QScrollArea *scrollArea;
    RawImage image_a; // reference image
    RawImage image_b; // "new" image
    std::vector<Feature> mFeatures;

    QString imageDirectory;
    bool trackingDirectory;
    QTimer *timer;
    std::map<QString, QDateTime> old_files;

    // Required by graph
    QtCharts::QChartView* mChartView;
    QtCharts::QLineSeries* mSerie_vec_x; // For X deviation in between images
    QtCharts::QLineSeries* mSerie_vec_y; // For Y deviation in between images
    QtCharts::QValueAxis *mAxisX_1;
    QtCharts::QValueAxis *mAxisY_1;
    qreal count_graph_entry = 6;
    float mChartMaxMin;
};
#endif // MAINWINDOW_H
