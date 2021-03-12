#include "qlabelimage.h"

#include <QMouseEvent>
#include <iostream>

LabelImage::LabelImage(RawImage *origin)
{
    mOririn = origin;
    zoomedLabel = NULL;
}

QImage LabelImage::QImageSubset(QImage* input) {
    QImage qimage = QPixmap(window_size, window_size).toImage();
    for(size_t x(0), x2(pointer.x()-window_size/2); x < window_size; x++, x2++) {
        for(size_t y(0), y2(pointer.y()-window_size/2); y < window_size; y++, y2++) {
            qimage.setPixelColor(x, y, input->pixelColor(x2 ,y2 ));
        }
    }
    return qimage;
}

void LabelImage::drawPointer() {
    this->setPixmap(savedMap); // clear background image
    MainWindow::stretchImage(this, 30);
    QImage tmp = this->pixmap()->toImage();
    if (zoomedLabel != NULL && mOririn != NULL && mOririn->bw != NULL) {
        // TODO: create subset from this->pixmap() because mOrigin will not exist animore
        //if (MainWindow::RawToQImageRectBW(mOririn, &qimage, getSelectionRect())) // Black and White
            //MainWindow::RawToQImageRect(mOririn, &qimage, getSelectionRect()); // Color
            //zoomedLabel->setPixmap(QPixmap::fromImage(qimage));
        zoomedLabel->setPixmap(QPixmap::fromImage(QImageSubset(&tmp)));
    }

    QPainter qPainter(&tmp);
    QPen pen(Qt::red);
    qPainter.setBrush(Qt::NoBrush);
    pen.setWidth(1);
    qPainter.setPen(pen);
    qPainter.drawPoint(pointer);
    qPainter.drawRect(pointer.x()-window_size/2, pointer.y()-window_size/2, window_size, window_size);
    qPainter.end();
    this->setPixmap(QPixmap::fromImage(tmp));
}

void LabelImage::mousePressEvent ( QMouseEvent * event ) {
    if (event->button() == Qt::LeftButton) {
        float x = this->pixmap()->width()*(event->localPos().x()/(float)this->size().width());
        float y = this->pixmap()->height()*(event->localPos().y()/(float)this->size().height());
        pointer = QPointF(x, y);
        drawPointer();
    }
}

QRect LabelImage::getSelectionRect() {
    return QRect(pointer.x()-window_size/2, pointer.y()-window_size/2, window_size, window_size);
}

void LabelImage::moveRect(qreal x, qreal y) {
    pointer.setX(x);
    pointer.setY(y);
}

void LabelImage::fromRaw(RawImage* raw) {
    if (raw->red == NULL || raw->blue == NULL || raw->green == NULL) {
        std::cout<<"Error: MainWindow::RawToQImage: Raw image is empty"<<std::endl;
        return;
    }

    QImage qimage = QPixmap(raw->width, raw->height).toImage();

    for(size_t x(0); x < raw->width; x++) {
        for(size_t y (0); y < raw->height; y++) {
            qimage.setPixelColor(x, y, QColor(raw->red[y*raw->width + x], raw->green[y*raw->width + x], raw->blue[y*raw->width + x]));
        }
    }
    savedMap = QPixmap::fromImage(qimage);
    this->setPixmap(savedMap);
    MainWindow::stretchImage(this, 30);

    this->adjustSize();
    this->setScaledContents(true);
}
