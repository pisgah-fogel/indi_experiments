#ifndef QLABELIMAGE_H
#define QLABELIMAGE_H

#include <QLabel>
#include "mainwindow.h"

class RawImage;

class LabelImage: public QLabel
{
public:
    LabelImage();
    void mousePressEvent ( QMouseEvent * event ) override;
    void fromRaw(RawImage* raw);
    void drawPointer();
    QPointF pointer;
    unsigned int window_size = 128;
};

#endif // QLABELIMAGE_H
