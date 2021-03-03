#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
private:
    static bool openFit(QString filename, QImage* image);
    Ui::MainWindow *ui;
    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1;
};
#endif // MAINWINDOW_H
