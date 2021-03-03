#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class RawImage {
public:
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
    void stretchImage(float intensity);
private slots:
    void callback_openFile();
    void callback_openFile_compare();
    void drawDebug();
private:
    void stackImageWithNewImage();
    static bool openFit(QString filename, RawImage *image);
    static void RawToQImage(RawImage* raw, QImage* qimage);
    static void computeBWfromRawImage(RawImage* rawimg);
    Ui::MainWindow *ui;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1;
    RawImage image_a; // reference image
    RawImage image_b; // "new" image
};
#endif // MAINWINDOW_H
