#include "TrustManager.h"
#include <iostream>
#include <stdlib.h>
#include <algorithm>

#include <inet/networklayer/ipv4/Ipv4Header_m.h>
#include "inet/mobility/contract/IMobility.h"
#include "common/TrustData.h"


// using namespace std;
// using namespace inet;

std::ostream& operator<<(std::ostream &os, const struct trustNode &node) {
    std::cout << "NodeId: " << node.nodeId << ", directReputation: " << node.directReputation << "\n";
    return os;
}

// void TrustManager::print() {
//     TrustNodeList *tmp = this->head;
//     while(tmp != nullptr) {
//         std::cout << *tmp << "\n";
//         tmp = tmp->next;
//     }
// }

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

TrustNodeList* TrustManager::addEntryTrustMap(string id, double directReputation, double lastRepo) {
    TrustNodeList *tmp = new TrustNodeList(id, directReputation, lastRepo);
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

void TrustManager::updateTrustValue(std::string sender, double directReputation, double lastRepo, int lastUpdatedTimestamp) {
    auto tmp = this->getTrustNode(sender);
    if(tmp == nullptr) {
        this->addEntryTrustMap(sender, directReputation, lastRepo);
    }
    else {
        //Update only if this is a newer message
        if(tmp->timestamp < lastUpdatedTimestamp) {
            tmp->directReputation = directReputation;
            tmp->lastCalculatedReputation = lastRepo;
        }
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

void updateRecommendedReputation(std::list<TrustManager *> &trustList, TrustData *msg, std::string sender, 
                                std::string rsuID) {
    TrustManager *senderManager = findVehicleInList(trustList, sender);

    for(auto j: trustList) {
        //Make sure we don't look at sender's ID(i) and rsuID(j)
        if(j->id.compare(sender) != 0 && j->id.compare(rsuID) != 0) {
            TrustNodeList *direct_reputation_i_j = senderManager->getTrustNode(j->id);
            TrustNodeList *direct_reputation_j_k = j->getTrustNode(rsuID);
            if(direct_reputation_i_j == nullptr) {
                //trust list of node i does not have entry corresponding to k
                //skip this i, k combination from the calculation for now and 
                //do leave computation for next time
                // std::cout << "Could not find reputation of k in i's list\n";
                continue;
            }

            if(direct_reputation_j_k == nullptr) {
                //trust list of node k does not have entry corresponding to j
                //skip this k, j combination from the calculation for now and 
                //do leave computation for next time
                // std::cout << "Could not find reputation of j in k's list\n";
                continue;
            }

            double reputation_diff = abs(direct_reputation_j_k->lastCalculatedReputation - direct_reputation_i_j->lastCalculatedReputation);
            if(direct_reputation_i_j->lastCalculatedReputation == 0) {
                direct_reputation_i_j->lastCalculatedReputation = 0.5;
            }
            else if(reputation_diff >= REPUTATION_DIFF_DELTA) {
                direct_reputation_i_j->lastCalculatedReputation -= reputation_diff * direct_reputation_i_j->lastCalculatedReputation;
            }
            else if(reputation_diff < REPUTATION_DIFF_DELTA) {
                direct_reputation_i_j->lastCalculatedReputation += (reputation_diff*(1-direct_reputation_i_j->lastCalculatedReputation))/8;
            }
        }
    }
}

void updateRecommendedReputationOfRecommender(std::list<TrustManager *> &trustList, TrustData *msg, std::string sender, 
                                std::string rsuID, std::string recommender) {
    TrustManager *senderManager = findVehicleInList(trustList, sender);
    TrustManager *rsuManager = findVehicleInList(trustList, rsuID);
    TrustManager *recommenderManager = findVehicleInList(trustList, recommender);

    // for(auto j: trustList) {
        //Make sure we don't look at sender's ID(k) and rsuID/receiver(i)
        //Here recommender is j
    if(recommender.compare(sender) != 0 && recommender.compare(rsuID) != 0) {
        TrustNodeList *reputation_i_j = rsuManager->getTrustNode(recommender);
        TrustNodeList *reputation_j_k = recommenderManager->getTrustNode(sender);
        TrustNodeList *reputation_i_k = rsuManager->getTrustNode(sender);
        if(reputation_i_j == nullptr) {
            //trust list of node i does not have entry corresponding to k
            //skip this i, k combination from the calculation for now and 
            //do leave computation for next time
            // std::cout << "Could not find reputation of k in i's list\n";
            return;
        }

        if(reputation_j_k == nullptr) {
            //trust list of node k does not have entry corresponding to j
            //skip this k, j combination from the calculation for now and 
            //do leave computation for next time
            // std::cout << "Could not find reputation of j in k's list\n";
            return;
        }

        if(reputation_i_k == nullptr) {
            //trust list of node k does not have entry corresponding to j
            //skip this k, j combination from the calculation for now and 
            //do leave computation for next time
            // std::cout << "Could not find reputation of j in k's list\n";
            return;
        }

        double reputation_diff = abs(reputation_j_k->lastCalculatedReputation - reputation_i_k->lastCalculatedReputation);
        if(reputation_i_j->lastCalculatedReputation == 0) {
            reputation_i_j->lastCalculatedReputation = 0.5;
        }
        else if(reputation_diff >= REPUTATION_DIFF_DELTA) {
            reputation_i_j->lastCalculatedReputation -= reputation_diff * reputation_i_j->lastCalculatedReputation;
        }
        else if(reputation_diff < REPUTATION_DIFF_DELTA) {
            reputation_i_j->lastCalculatedReputation += (reputation_diff*(1-reputation_i_j->lastCalculatedReputation))/8;
        }
    }
    // }
}

// double getSimilarity(std::unordered_map<std::string, veins::VeinsInetMobility *> &mobilityMap,
//                                 std::string k_car,
//                                 std::string rsu_ID) {
//     inet::Coord sender_velocity = mobilityMap[rsu_ID]->getCurrentVelocity();

//     //TODO: Change this to velocity of all other nodes in the simulation
//     inet::Coord curr_velocity;
    
//     if(!k_car.compare(evilVehicleID)) {
//         curr_velocity = Coord(1000, 1000, 1000);
//     }
//     else {
//         curr_velocity = mobilityMap[k_car]->getCurrentVelocity();
//     }

//     int DIMS = 3;
//     double dist = curr_velocity.sqrdist(sender_velocity);
//     return DIMS/dist;
// }

// double getRecommendedReputation(std::list<TrustManager *> &trustList, TrustData *msg, std::string sender, 
//             std::unordered_map<std::string, veins::VeinsInetMobility *> &mobilityMap, std::string rsuID) {;
//     TrustManager *senderManager = findVehicleInList(trustList, sender);

//     double numerator = 0.0;
//     double denominator = 0.0;
//     for(auto k: trustList) {
//         //Make sure we don't look at sender's ID(i) and rsuID(j)
//         if(k->id.compare(sender) != 0 && k->id.compare(rsuID) != 0) {
//             TrustNodeList *direct_reputation_i_k = senderManager->getTrustNode(k->id);
//             TrustNodeList *direct_reputation_k_j = k->getTrustNode(rsuID);
//             if(direct_reputation_i_k == nullptr) {
//                 //trust list of node i does not have entry corresponding to k
//                 //skip this i, k combination from the calculation for now and 
//                 //do leave computation for next time
//                 // std::cout << "Could not find reputation of k in i's list\n";
//                 continue;
//             }

//             if(direct_reputation_k_j == nullptr) {
//                 //trust list of node k does not have entry corresponding to j
//                 //skip this k, j combination from the calculation for now and 
//                 //do leave computation for next time
//                 // std::cout << "Could not find reputation of j in k's list\n";
//                 continue;
//             }

//             double r_i_k = direct_reputation_i_k->lastCalculatedReputation;
//             double r_k_j = direct_reputation_k_j->lastCalculatedReputation;
//             // double similarity = msg->getSimilarity(mobilityMap, k->id);
//             double similarity = getSimilarity(mobilityMap, k->id, rsuID); // try changing it to 1
//             // numerator += similarity* r_i_k * r_k_j;
//             // denominator += similarity * r_i_k;
//             numerator += 1* r_i_k * r_k_j;
//             denominator += 1 * r_i_k;
//         }
//     }
//     if(denominator == 0.0) {
//         return 0.0;
//     }
//     //Dividing by 1 because dividing by the denominator does not make sense
//     //since that would give a recommended reputation of 1 even if similarity b/w
//     //sender and recommender is really really low
//     return numerator / denominator;
// }

double getDirectReputation(TrustManager *list, std::string sender) {
    auto trustOfNode =  list->getTrustNode(sender);
    if(!trustOfNode) {
        EV << "Could not find trust value for the sender: |" << sender << "|\n";
        return 0.0;
    }
    else {
        return trustOfNode->directReputation;
    }
}
