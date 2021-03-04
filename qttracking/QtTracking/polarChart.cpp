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

#define POLAR_SIZE 150
#define POLAR_SQUARE_SIZE 5
#define POLAR_AMPLIFICATION 10
#define POLAR_LINE_WIDTH 3

void MainWindow::createPolarChart()
{
    ui->polar->setBackgroundRole(QPalette::Base);
    ui->polar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QImage tmp = QPixmap(POLAR_SIZE, POLAR_SIZE).toImage();
    QPainter qPainter(&tmp);
    QPen pen(Qt::green);
    pen.setWidth(POLAR_LINE_WIDTH);
    qPainter.setPen(pen);
    qPainter.drawPoint(POLAR_SIZE/2,POLAR_SIZE/2);
    qPainter.drawRect(POLAR_SIZE/2-POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE,
                      POLAR_SIZE/2-POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE,
                      POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE*2,
                      POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE*2);
    qPainter.end();
    ui->polar->setPixmap(QPixmap::fromImage(tmp));
}

void MainWindow::addPointToPolarChart(float x, float y)
{
    // QImage tmp = ui->polar->pixmap()->toImage(); // TODO: Stack ?
    QImage tmp = QPixmap(POLAR_SIZE, POLAR_SIZE).toImage();
    QPainter qPainter(&tmp);
    QPen pen(Qt::green);
    pen.setWidth(POLAR_LINE_WIDTH);
    qPainter.setPen(pen); // qPainter has a copy of the pen
    qPainter.drawPoint(POLAR_SIZE/2,POLAR_SIZE/2);
    qPainter.drawRect(POLAR_SIZE/2-POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE,
                      POLAR_SIZE/2-POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE,
                      POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE*2,
                      POLAR_AMPLIFICATION*POLAR_SQUARE_SIZE*2);
    pen.setColor(Qt::red);
    qPainter.setPen(pen); // Update qPainter's copy
    //qPainter.drawPoint((int)x,(int)y);
    qPainter.drawLine(POLAR_SIZE/2, POLAR_SIZE/2, (int)((float)POLAR_AMPLIFICATION*x)+POLAR_SIZE/2,(int)((float)POLAR_AMPLIFICATION)*(int)y+POLAR_SIZE/2);
    qPainter.end();
    ui->polar->setPixmap(QPixmap::fromImage(tmp));
    ui->polar->update();
}
