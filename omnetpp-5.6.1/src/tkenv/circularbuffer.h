//==========================================================================
//  CIRCULARBUFFER.H - part of
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

#ifndef __OMNETPP_TKENV_CIRCULARBUFFER_H
#define __OMNETPP_TKENV_CIRCULARBUFFER_H

#include "tkdefs.h"
#include "omnetpp/simkerneldefs.h" // for ASSERT

namespace omnetpp {
namespace tkenv {

/**
 * A limited STL-like circular buffer implementation.
 */
template <typename T>
class circular_buffer
{
    private:
        T *cb;              // size of the circular buffer
        int cbsize;         // always power of 2
        int cbhead, cbtail; // cbhead is inclusive, cbtail is exclusive

    private:
        void grow() {
            ASSERT(cbhead == cbtail); // here it means full, not empty

            int newsize = 2*cbsize;
            T *newcb = new T[newsize];

            memcpy(newcb, cb + cbhead, (cbsize - cbhead)*sizeof(T));
            memcpy(newcb + (cbsize - cbhead), cb, cbtail*sizeof(T));

            delete[] cb;
            cb = newcb;

            cbhead = 0;
            cbtail = cbsize;
            cbsize = newsize;
        }

        // unimplemented:
        circular_buffer(const circular_buffer&);
        void operator=(const circular_buffer&);

    public:
        circular_buffer(int capacity=32) :
            cb(new T[capacity]), cbsize(capacity), cbhead(0), cbtail(0) { }
        ~circular_buffer() {
            delete[] cb;
        }
        void push_back(T d) {
            cb[cbtail] = d;
            cbtail = (cbtail + 1) & (cbsize-1);
            if (cbtail == cbhead)
                grow();
        }
        void pop_front() {
            ASSERT(cbhead != cbtail);
            cbhead = (cbhead + 1) & (cbsize-1);
        }
        T front() const {
            ASSERT(cbhead != cbtail);
            return cb[cbhead];
        }
        T back() const {
            ASSERT(cbhead != cbtail);
            return cb[(cbtail - 1) & (cbsize-1)];
        }
        T operator[](int i) const {
            ASSERT(cbhead != cbtail);
            return cb[(cbhead + i) & (cbsize-1)];
        }
        int size() const {
            return (cbtail - cbhead) & (cbsize-1);
        }
        bool empty() const {
            return cbhead == cbtail;
        }
        void clear() {
            cbhead = cbtail = 0;
        }
};

} // namespace tkenv
}  // namespace omnetpp

#endif

