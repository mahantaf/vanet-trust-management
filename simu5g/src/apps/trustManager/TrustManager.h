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
    // friend bool operator== ( const TrustManager &n1, const TrustManager &n2);
    bool operator== (TrustManager const *r) const {
        return this->id.compare(r->id) ==0;
    }

    public:
        string id;
        TrustNodeList* head;
        TrustManager *next;
        TrustManager(string id):
            id(id),
            head(nullptr),
            next(nullptr)
        { };

        void print();
        TrustNodeList *getTrustMap();
        TrustNodeList* addEntryTrustMap(string id, double reputation);
        double getTrustValue(Packet *pkt);
        TrustNodeList *getTrustNode(std::string sender);
        void updateTrustValue(Packet *pkt);
        void updateTrustValue(double newTrust, std::string sender);
        
};


struct TrustManagerComparer {
    bool operator() (const TrustManager *l, const TrustManager *r) const {
        return l->id.compare(r->id);
    }
};

TrustManager *findVehicleInList(std::list<TrustManager*> &trustList, std::string nodeId);
double getReputation(TrustManager *list, std::string sender);
// bool doTrustManagement(std::vector<std::pair<std::string, TrustData *>> &messagesReceived, std::list<TrustManager *> &trustListAllVehicles,
//          TrustData *msg, inet::IMobility *mobility, bool rsuID);

// /* Contains trust management information stored by all vehicles */
// class TrustManagerAllVehicles {
//     TrustManager *head;

//     void addTrustManager(TrustManager *newVehicle);
// };
#endif // #ifndef _TRUST_MANAGER_H