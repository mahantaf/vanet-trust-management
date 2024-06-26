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

#ifndef _VOIPSENDER_H_
#define _VOIPSENDER_H_

#include <string.h>
#include <omnetpp.h>

#include <inet/networklayer/common/L3AddressResolver.h>
#include <inet/transportlayer/contract/udp/UdpSocket.h>
#include "apps/voip/VoipPacket_m.h"
#include "apps/voip/ConstantEventLocationGenerator.h"
#include "inet/mobility/contract/IMobility.h"
#include "veins_inet/VeinsInetMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

#include <unordered_set>
#include <unordered_map>

#include "veins_inet/VeinsInetMobility.h"

/* =========================================================
 * NOTE: Don't add spaces after comma when adding more cars
 *       to the list
 * =========================================================
 */
#define evilVehicleID "car[0],car[1],car[2],car[3]"
#define ENABLE_SENSOR_RANGE 1
#define SENSOR_START_Y 191
#define SENSOR_END_Y 310
#define SENSOR_START_X 380
#define SENSOR_END_X 490

class VoIPSender : public omnetpp::cSimpleModule
{
    inet::UdpSocket socket;

    //source
    omnetpp::simtime_t durTalk_;
    omnetpp::simtime_t durSil_;
    double scaleTalk_;
    double shapeTalk_;
    double scaleSil_;
    double shapeSil_;
    bool isTalk_;
    omnetpp::cMessage* selfSource_;
    //sender
    int iDtalk_;
    int nframes_;
    int iDframe_;
    int nframesTmp_;
    int size_;
    omnetpp::simtime_t sampling_time;

    bool silences_;

    unsigned int totalSentBytes_;
    omnetpp::simtime_t warmUpPer_;

    omnetpp::simsignal_t voIPGeneratedThroughtput_;
    // ----------------------------

    omnetpp::cMessage *selfSender_;

    omnetpp::cMessage *initTraffic_;

    omnetpp::simtime_t timestamp_;
    int localPort_;
    int destPort_;
    inet::L3Address destAddress_;

    int numMessages;
    bool firstMessage;

    /* Mobility information(Addition by Nishchay)*/
    cModule* ue;
    veins::VeinsInetMobility *mobility;
    ConstantEventLocationGenerator eventLocationGenerator; 

    void initTraffic();
    void talkspurt(omnetpp::simtime_t dur);
    void selectPeriodTime();
    void sendVoIPPacket();

  public:
    ~VoIPSender();
    VoIPSender();

  protected:

    virtual int numInitStages() const  override { return inet::NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessage(omnetpp::cMessage *msg) override;

};

#endif

