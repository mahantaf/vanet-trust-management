//========================================================================
//  CEVENT.CC - part of
//
//                 OMNeT++/OMNEST
//              Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <cstdio>  // sprintf
#include <sstream>
#include "omnetpp/globals.h"
#include "omnetpp/cevent.h"
#include "omnetpp/csimulation.h"
#include "omnetpp/cexception.h"
#include "omnetpp/cenvir.h"

#ifdef WITH_PARSIM
#include "omnetpp/ccommbuffer.h"
#endif

using namespace omnetpp;

using std::ostream;

cEvent::cEvent(const cEvent& event) : cOwnedObject(event)
{
    heapIndex = -1;
    insertOrder = -1;
    previousEventNumber = -1;
    operator=(event);
}

cEvent::cEvent(const char *name) : cOwnedObject(name, false)
{
    // name pooling is off for messages by default, as unique names are quite common
    priority = 0;
    arrivalTime = 0;
    heapIndex = -1;
    insertOrder = -1;
    previousEventNumber = -1;
}

cEvent::~cEvent()
{
}

std::string cEvent::str() const
{
    if (arrivalTime == getSimulation()->getSimTime())
        return "(now)";
    if (arrivalTime < getSimulation()->getSimTime())
        return "(in the past)";

    std::stringstream out;
    simtime_t dt = arrivalTime - getSimulation()->getSimTime();
    out << "at t=" << arrivalTime << ", in dt=" << dt.ustr();
    if (getTargetObject())
        out << ", for " << getTargetObject()->getFullPath();
    return out.str();
}

void cEvent::forEachChild(cVisitor *v)
{
}

void cEvent::parsimPack(cCommBuffer *buffer) const
{
#ifndef WITH_PARSIM
    throw cRuntimeError(this, E_NOPARSIM);
#else
    cOwnedObject::parsimPack(buffer);

    buffer->pack(priority);
    buffer->pack(arrivalTime);
    buffer->pack(heapIndex);
    buffer->pack(insertOrder);
#endif
}

void cEvent::parsimUnpack(cCommBuffer *buffer)
{
#ifndef WITH_PARSIM
    throw cRuntimeError(this, E_NOPARSIM);
#else
    cOwnedObject::parsimUnpack(buffer);

    buffer->unpack(priority);
    buffer->unpack(arrivalTime);
    buffer->unpack(heapIndex);
    buffer->unpack(insertOrder);
#endif
}

cEvent& cEvent::operator=(const cEvent& event)
{
    if (this == &event)
        return *this;

    cOwnedObject::operator=(event);

    priority = event.priority;
    arrivalTime = event.arrivalTime;

    return *this;
}

bool cEvent::shouldPrecede(const cEvent *other) const
{
    return getArrivalTime() < other->getArrivalTime() ? true :
           getArrivalTime() > other->getArrivalTime() ? false :
           getSchedulingPriority() == other->getSchedulingPriority() ? getInsertOrder() < other->getInsertOrder() :
           getSchedulingPriority() < other->getSchedulingPriority() ? true :
           getSchedulingPriority() > other->getSchedulingPriority() ? false :
           getInsertOrder() < other->getInsertOrder();
}

int cEvent::compareBySchedulingOrder(const cEvent *a, const cEvent *b)
{
    if (a->getArrivalTime() < b->getArrivalTime())
        return -1;
    if (a->getArrivalTime() > b->getArrivalTime())
        return 1;

    int priorityDiff = a->getSchedulingPriority() - b->getSchedulingPriority();
    if (priorityDiff)
        return priorityDiff;

    return a->getInsertOrder() < b->getInsertOrder() ? -1 : 1; // they cannot be equal
}
