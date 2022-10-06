#include "TrustManager.h"
#include <iostream>
#include <stdlib.h>
#include <algorithm>

#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "inet/mobility/contract/IMobility.h"
#include "common/TrustData.h"


std::ostream& operator<<(std::ostream &os, const struct trustNode &node) {
    std::cout << "NodeId: " << node.nodeId << ", reputation: " << node.reputation << "\n";
    return os;
}
TrustNodeList* TrustManager::getTrustMap() {
    return this->head;
}

TrustNodeList *TrustManager::getTrustNode(std::string sender){
    TrustNodeList *tmp = this->head;
    while(tmp != nullptr) {
        if(!tmp->nodeId.compare(sender)) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return tmp;
}

double TrustManager::getTrustValue(Packet *pkt) {
    // L3Address src;
    // L3AddressTagBase *addresses= pkt->findTag<L3AddressReq>();
    // src = addresses->getSrcAddress();
    const auto& hdr = pkt->peekAtFront<Ipv4Header>();
    const Ipv4Address& src = hdr->getSrcAddress();

    TrustNodeList *tmp = this->head;
    while(tmp != nullptr) {
        if(tmp->nodeId.compare(src.str()) == 0) {
            break;
        }
        tmp = tmp->next;
    }
    if(tmp != nullptr) {
        return tmp->reputation;
    }
    /* This is not possible since we would have added the entry 
       to the trustmap through updateTrustMap() before calling 
       this function
     */
    return NO_TRUST;
}

TrustNodeList* TrustManager::addEntryTrustMap(string id, double reputation) {
    TrustNodeList *tmp = new TrustNodeList(id, reputation);
    if(this->head == nullptr) {
        this->head = tmp;
    }
    else {
        /* Reach to the end of list of trustnodes */
        TrustNodeList *ptr = this->head;
        while(ptr->next != nullptr) {
            ptr = ptr->next;
        }
        ptr->next = tmp;
    }
    return this->head;
}

void TrustManager::updateTrustValue(Packet *pkt) {
    // L3Address src, dst;
    // L3AddressTagBase *addresses= pkt->findTag<L3AddressReq>();
    // src = addresses->getSrcAddress();
    const auto& hdr = pkt->peekAtFront<Ipv4Header>();
    const Ipv4Address& src = hdr->getSrcAddress();

    TrustNodeList *tmp = this->head;
    while(tmp != nullptr) {
        if(tmp->nodeId.compare(src.str()) == 0) {
            break;
        }
        tmp = tmp->next;
    }
    if(tmp != nullptr) {
        /* Dummy value decreasing for every packet received */
        if(tmp->reputation > TRUST_THRESHOLD) {
            tmp->reputation -= 10;
        }
    }
    else {
        this->head = addEntryTrustMap(src.str(), DEFAULT_TRUST);
    }
}

// bool operator==(TrustManager &tm1, TrustManager &tm2) {
//     return (tm1.id.compare(tm2.id) == 0);
// }

TrustManager *findVehicleInList(std::list<TrustManager*> &trustList, std::string nodeId) {
    auto tmpItr = trustList.begin();
    while(tmpItr != trustList.end()) {
        if((*tmpItr)->id.compare(nodeId) == 0) {
            return *tmpItr;
        }
        tmpItr++;
    }
    return nullptr;
}

// override pair's == operator to compare sender and trustMsg correctly
//Changed to template<class T> since using std::pair<string, TrustData *> was giving error
// template<class T>
// bool operator==(const std::pair<std::string, T>& lhs, const std::pair<std::string, T>& rhs) {
//     return (lhs.first.compare(rhs.first) == 0 && *(lhs.second) == *(rhs.second));    
// };

double getReputation(TrustManager *list, std::string sender) {
    auto trustOfNode =  list->getTrustNode(sender);
    if(!trustOfNode) {
        EV << "Could not find trust value for the sender: |" << sender << "|\n";
        return 0.0;
    }
    else {
        return trustOfNode->reputation;
    }
}
