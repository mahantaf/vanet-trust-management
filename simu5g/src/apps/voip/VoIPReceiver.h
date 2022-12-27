//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_VOIPRECEIVER_H_
#define _LTE_VOIPRECEIVER_H_

#include <list>
#include <string.h>
#include <unordered_set>

#include <omnetpp.h>

#include <inet/networklayer/common/L3AddressResolver.h>
#include <inet/transportlayer/contract/udp/UdpSocket.h>

#include "apps/voip/VoipPacket_m.h"
#include "apps/trustManager/TrustManager.h"
#include "common/TrustData.h"

#include "apps/voip/Remote_Attestation_m.h"

//(20^2 since the distance calculation function returns dist^2 and it's easier to do it this way)
#define ERROR_MARGIN 400 
#define PROBATION 1
#define MIN_NODES_FOR_REPUTATION_UPDATE 4
#define ALGO_3_2 1 // To enable reputation updating when collective data of MIN_NODES_FOR_REPUTATION_UPDATE nodes is there(Algorithm 2 in overleaf doc)
#define MAP_RANGE 800

// Point class for k-means clustering for clustering based location estimation
struct Point {
    double x, y;     // coordinates
    int cluster;     // no default cluster
    double minDist;  // default infinite dist to nearest cluster
    int nPoints;

    Point() : 
        x(0.0), 
        y(0.0),
        cluster(-1),
        minDist(__DBL_MAX__),
        nPoints(0) {}
        
    Point(double x, double y) : 
        x(x), 
        y(y),
        cluster(-1),
        minDist(__DBL_MAX__),
        nPoints(0) {}

    double distance(Point p) {
        return (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y);
    }
};
vector<Point> kMeansClustering(vector<Point>* points, int epochs, int k);

class VoIPReceiver : public omnetpp::cSimpleModule
{
    inet::UdpSocket socket;

    ~VoIPReceiver();

    int emodel_Ie_;
    int emodel_Bpl_;
    int emodel_A_;
    double emodel_Ro_;

    typedef std::list<VoipPacket*> PacketsList;
    PacketsList mPacketsList_;
    PacketsList mPlayoutQueue_;
    unsigned int mCurrentTalkspurt_;
    unsigned int mBufferSpace_;
    omnetpp::simtime_t mSamplingDelta_;
    omnetpp::simtime_t mPlayoutDelay_;

    bool mInit_;

    unsigned int totalRcvdBytes_;
    omnetpp::simtime_t warmUpPer_;

    omnetpp::simsignal_t voIPFrameLossSignal_;
    omnetpp::simsignal_t voIPFrameDelaySignal_;
    omnetpp::simsignal_t voIPPlayoutDelaySignal_;
    omnetpp::simsignal_t voIPMosSignal_;
    omnetpp::simsignal_t voIPTaildropLossSignal_;
    omnetpp::simsignal_t voIPPlayoutLossSignal_;
    omnetpp::simsignal_t voIPJitterSignal_;
    omnetpp::simsignal_t voIPReceivedThroughput_;


    RemoteAttestationMsg *remoteAttestationDoneMsg_;
    omnetpp::simsignal_t alertSendMsgSignal_;
    TrustManager* trustListAllVehicles;
    std::unordered_map<std::string, TrustData *> messagesReceived;

    //Adding statistics

    virtual void finish() override;

  protected:

    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessage(omnetpp::cMessage *msg) override;
    double eModel(omnetpp::simtime_t delay, double loss);
    void playout(bool finish);

  //Trust management
  public:
    std::list<TrustManager *> trustListAllVehicles;
    std::unordered_map<std::string, TrustData *> messagesReceived;

    std::unordered_set<std::string> rsuSet;
    int numRecvd;
    void emitAppropriateReputationSignal(std::string sender, int sigVal);
    bool doTrustManagement(std::unordered_map<std::string, TrustData *> &messagesReceived, TrustData *msg, std::string rsuID,
                            VoIPReceiver *recvr_class); 

    void doRemoteAttestation(std::string sender);
};

#endif

