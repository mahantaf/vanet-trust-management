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

TrustNodeList *TrustManager::getTrustNode(std::string sender){
    for(auto trustNode: this->trustNodes) {
        if(!trustNode->nodeId.compare(sender)) {
            return trustNode;
        }
    }
    return nullptr;
}

TrustNodeList* TrustManager::addEntryTrustMap(string id, double reputation) {
    TrustNodeList *tmp = new TrustNodeList(id, reputation);
    this->trustNodes.push_back(tmp);
    return tmp;
}

double TrustManager::getReputation(std::string sender) {
    auto trustOfNode =  this->getTrustNode(sender);
    if(!trustOfNode) {
        EV << "Could not find trust value for the sender: |" << sender << "|\n";
        return 0.0;
    }
    else {
        return trustOfNode->reputation;
    }
}
