//==========================================================================
//  VECTORPLOTITEM.H - part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __OMNETPP_QTENV_VECTORPLOTITEM_H
#define __OMNETPP_QTENV_VECTORPLOTITEM_H

#include <QGraphicsItem>
#include "outputvectorinspector.h"

namespace omnetpp {
namespace qtenv {

class VectorPlotItem : public QGraphicsItem
{
  public:
    enum PlottingMode {
        DRAW_DOTS = 0,
        DRAW_POINTS = 1,
        DRAW_PINS = 2,
        DRAW_BARS = 3,
        DRAW_SAMPLEHOLD = 4,
        DRAW_BACKWARD_SAMPLEHOLD = 5,
        DRAW_LINES = 6
    };

  private:
    QColor color;
    const CircBuffer *circBuf;
    PlottingMode plottingMode;
    double minX, maxX, minY, maxY;

    int mapFromSimtime(const simtime_t& t);
    int mapFromValue(const double y);

  public:
    VectorPlotItem(QGraphicsItem *parent = nullptr);

    void setMinX(const double minX);
    void setMaxX(const double maxX);
    void setMinY(const double minY);
    void setMaxY(const double maxY);

    void setData(const CircBuffer *circBuf);
    void setColor(const QColor& color);

    PlottingMode getPlottingMode() const;
    void setPlottingMode(const PlottingMode plottingMode);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    QRectF boundingRect() const override;
};

}  // namespace qtenv
}  // namespace omnetpp

#endif

