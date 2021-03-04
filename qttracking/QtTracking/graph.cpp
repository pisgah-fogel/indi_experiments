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


void MainWindow::createGraph() {
    mAxisX_1 = new QtCharts::QValueAxis();
    mAxisY_1 = new QtCharts::QValueAxis();
    mChartView = new QtCharts::QChartView();
    mSerie_vec_x = new QtCharts::QLineSeries();
    mChartView->chart()->addSeries(mSerie_vec_x);
    mSerie_vec_y = new QtCharts::QLineSeries();
    mChartView->chart()->addSeries(mSerie_vec_y);
    //chartView->chart()->createDefaultAxes();
    mChartView->chart()->addAxis(mAxisX_1,Qt::AlignBottom);
    mChartView->chart()->addAxis(mAxisY_1,Qt::AlignLeft);
    mSerie_vec_x->attachAxis(mAxisX_1);
    mSerie_vec_x->attachAxis(mAxisY_1);
    mSerie_vec_y->attachAxis(mAxisX_1);
    mSerie_vec_y->attachAxis(mAxisY_1);
    mAxisX_1->setTickCount(21);
    mAxisX_1->setRange(0, 10);
    mAxisY_1->setRange(-20, 20);

    // Add some datas
    mSerie_vec_x->append(0, 0);
    mSerie_vec_x->append(1, 0);
    mSerie_vec_x->append(2, 0);
    mSerie_vec_x->append(3, 0);
    mSerie_vec_x->append(4, 0);
    mSerie_vec_x->append(5, 0);
    mSerie_vec_y->append(0, 0);
    mSerie_vec_y->append(1, 0);
    mSerie_vec_y->append(2, 0);
    mSerie_vec_y->append(3, 0);
    mSerie_vec_y->append(4, 0);
    mSerie_vec_y->append(5, 0);

    ui->graphLayout->addWidget(mChartView);
}

void MainWindow::addValueToGraph(float vec_x, float vec_y) {
    // To add something to the series
    qreal x_scroll = mChartView->chart()->plotArea().width() / mAxisX_1->tickCount();
    qreal x_decal = (mAxisX_1->max() - mAxisX_1->min()) / mAxisX_1->tickCount();
    qreal m_x = 1 + x_decal;
    mSerie_vec_x->append(m_x, vec_x);
    mSerie_vec_y->append(m_x, vec_y);
    mChartView->chart()->scroll(x_scroll, 0);
    mChartView->update();
}
