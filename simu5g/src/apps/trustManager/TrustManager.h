#ifndef _TRUST_MANAGER_H
#define _TRUST_MANAGER_H

#include <stdio.h>
#include <omnetpp.h>
#include <string>

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "common/TrustData.h"

#define TRUST_MANAGEMENT_ENABLED 1

#define DEFAULT_TRUST .8f
#define NO_TRUST 0.f
#define TRUST_THRESHOLD 0.15f
#define DIRECT_REPUTATION_REWARD 1.1f
#define REPUTATION_DIFF_DELTA 0.05f
#define REMOTE_ATTESTATION_TIME 1

#define evilVehicleID "car[2]"

using namespace std;
using namespace inet;

typedef struct trustNode {
    string nodeId;
    double reputation;
    struct trustNode *next;

    trustNode(string id, double reputation):nodeId(id), 
            reputation(reputation), next(nullptr) {

    };

    friend std::ostream& operator<<(std::ostream &os, const struct trustNode &node);
}TrustNodeList;

class TrustManager {
    public:
        vector<TrustNodeList*> trustNodes;

        void print();
        TrustNodeList* addEntryTrustMap(string id, double reputation);
        TrustNodeList *getTrustNode(std::string sender);
        double getReputation(std::string sender);
};


#endif // #ifndef _TRUST_MANAGER_H