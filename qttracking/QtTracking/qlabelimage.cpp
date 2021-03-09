#include "qlabelimage.h"

#include <QMouseEvent>
#include <iostream>

LabelImage::LabelImage()
{

}

void LabelImage::drawPointer() {
    QImage tmp = this->pixmap()->toImage();
    QPainter qPainter(&tmp);
    QPen pen(Qt::red);
    qPainter.setBrush(Qt::NoBrush);
    pen.setWidth(10);
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

    this->setPixmap(QPixmap::fromImage(qimage));
}
