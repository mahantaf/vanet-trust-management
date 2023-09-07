//==========================================================================
//  COMPONENTHISTORY.H - part of
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

#ifndef __OMNETPP_TKENV_COMPONENTHISTORY_H
#define __OMNETPP_TKENV_COMPONENTHISTORY_H

#include <string>
#include <map>
#include "omnetpp/simutil.h"
#include "tkdefs.h"

namespace omnetpp {

class cComponentType;

namespace tkenv {

class TKENV_API ComponentHistory
{
    private:
        struct Data {
            int parentModuleId;
            opp_string fullName; //TODO stringpool it!
            cComponentType *componentType;
        };
        std::map<int,Data> components; // indexed by componentId

    private:
        const Data& lookup(int componentId) const {
            std::map<int,Data>::const_iterator it = components.find(componentId);
            ASSERT(it!=components.end());
            return it->second;
        }

    public:
        ComponentHistory() {}
        ~ComponentHistory() {}
        void componentDeleted(cComponent *component);
        void componentReparented(cComponent *component, cModule *oldParent, int oldId);
        int getParentModuleId(int componentId) const;
        const char *getComponentFullName(int componentId) const;
        std::string getComponentFullPath(int componentId) const;
        std::string getComponentRelativePath(int componentId, int rootComponentId) const;
        cComponentType *getComponentType(int componentId) const;
        std::string getComponentTypeName(int componentId) const;
        void clear();
};

} // namespace tkenv
}  // namespace omnetpp

#endif

