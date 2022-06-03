//Named this file TrustData.h due to compilation ordering issues
//happening when I named it TrustMsgContent.h and placed it
//in apps/trustManager/ folder


#ifndef _TRUST_MSG_CONTENT_H
#define _TRUST_MSG_CONTENT_H

#include <unordered_map>

#include <omnetpp.h>
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/MemoryInputStream.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/mobility/contract/IMobility.h"

#include "veins_inet/VeinsInetMobility.h"

using namespace inet;

class TrustData
{
    friend class VoIPSender;
    friend bool operator== ( const TrustData &n1, const TrustData &n2);
    omnetpp::simtime_t generation_time;
    inet::Coord generator_location;
    inet::Coord event_location;
    inet::Coord node_velocity;
    std::string senderID;

    public:
        TrustData(){};

        TrustData(const omnetpp::simtime_t time, inet::Coord gen_loc, inet::Coord ev_loc,
                        inet::Coord velocity, std::string senderID):
        generation_time(time), generator_location(gen_loc), 
        event_location(ev_loc), node_velocity(velocity), senderID(senderID) {

        };
        inet::Coord getEventLocation() {
            return event_location;
        }
        
        inet::Coord getGeneratorLocation() {
            return generator_location;
        }

        omnetpp::simtime_t getGenerationTime() {
            return generation_time;
        }

        inet::Coord getNodeVelocity() {
            return node_velocity;
        }

        std::string getSenderID() {
            return senderID;
        }

        void serializeTrustData(MemoryOutputStream &stream);
        void deserializeTrustData(MemoryInputStream& stream);
};


#endif //_TRUST_MSG_CONTENT_H