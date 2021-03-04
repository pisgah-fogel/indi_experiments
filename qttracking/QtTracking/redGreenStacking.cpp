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

void MainWindow::createRedGreenStacking() {
    ui->RedGreenStacking->setBackgroundRole(QPalette::Base);
    ui->RedGreenStacking->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->RedGreenStacking->setScaledContents(true);

    //ui->RedGreenStacking->setPixmap(QPixmap::fromImage(tmp));
}

void MainWindow::redGreenStackingChangeImage(QImage* image) {
    ui->RedGreenStacking->setPixmap(QPixmap::fromImage(*image));
    ui->RedGreenStacking->adjustSize();
    ui->RedGreenStacking->setScaledContents(true);
    stretchImage(ui->RedGreenStacking, 30);
}
