#ifndef QLABELIMAGE_H
#define QLABELIMAGE_H

#include <QLabel>
#include "mainwindow.h"

class RawImage;

class LabelImage: public QLabel
{
public:
    LabelImage(RawImage* origin);
    void mousePressEvent ( QMouseEvent * event ) override;
    void fromRaw(RawImage* raw);
    void drawPointer();
    QPointF pointer;
    unsigned int window_size = 64;
    QRect getSelectionRect();
    void setZoomedLabel(QLabel* label) {
        zoomedLabel = label;
    }
    void moveRect(qreal x, qreal y);
private:
    RawImage* mOririn;
    QLabel* zoomedLabel;
};

#endif // QLABELIMAGE_H
