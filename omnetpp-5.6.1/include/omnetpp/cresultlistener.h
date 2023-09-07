//==========================================================================
//  CRESULTLISTENER.H - part of
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

#ifndef __OMNETPP_CRESULTLISTENER_H
#define __OMNETPP_CRESULTLISTENER_H

#include "simkerneldefs.h"
#include "clistener.h"
#include "cobject.h"

namespace omnetpp {

class cResultFilter;
class cComponent;


/**
 * @brief Common abstract base class for result filters and result recorders.
 *
 * @ingroup SimSupport
 */
class SIM_API cResultListener : public cObject, public cIListener
{
        friend class cResultFilter;
    protected:
        static const char *getPooled(const char *s);

    protected:
        // simplified API that better supports chaining:
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) = 0;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, long l, cObject *details) = 0;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l, cObject *details) = 0;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) = 0;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) = 0;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) = 0;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) = 0;
        virtual void subscribedTo(cResultFilter *prev);
        virtual void unsubscribedFrom(cResultFilter *prev);
        virtual void callFinish(cResultFilter *prev) {finish(prev);}
        virtual void finish(cResultFilter *prev) {}

        // original listener API delegates to simplified API:
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, bool b, cObject *details) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, long l, cObject *details) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, unsigned long l, cObject *details) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, const SimTime& v, cObject *details) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, const char *s, cObject *details) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
        virtual void subscribedTo(cComponent *component, simsignal_t signalID) override;
        virtual void unsubscribedFrom(cComponent *component, simsignal_t signalID) override;
        virtual void finish(cComponent *component, simsignal_t signalID) override;

};

}  // namespace omnetpp

#endif


