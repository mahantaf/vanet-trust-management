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

#include <cmath>
#include <inet/common/TimeTag_m.h>
#include "apps/voip/VoIPSender.h"
#include "apps/voip/VoIPReceiver.h"
#include "apps/voip/ReputationListSerializer.h"

#include "common/TrustData.h"

std::string rsuIDs = "car[0],car[1]";

#define round(x) floor((x) + 0.5)

Define_Module(VoIPSender);
using namespace inet;

VoIPSender::VoIPSender()
{
    selfSource_ = nullptr;
    selfSender_ = nullptr;
}

VoIPSender::~VoIPSender()
{
    cancelAndDelete(selfSource_);
    cancelAndDelete(selfSender_);
}

void VoIPSender::initialize(int stage)
{
    EV << "VoIP Sender initialize: stage " << stage << endl;

    cSimpleModule::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    durTalk_ = 0;
    durSil_ = 0;
    selfSource_ = new cMessage("selfSource");
    scaleTalk_ = par("scale_talk");
    shapeTalk_ = par("shape_talk");
    scaleSil_ = par("scale_sil");
    shapeSil_ = par("shape_sil");
    isTalk_ = par("is_talk");
    iDtalk_ = 0;
    nframes_ = 0;
    nframesTmp_ = 0;
    iDframe_ = 0;
    timestamp_ = 0;
    size_ = par("PacketSize");
    sampling_time = par("sampling_time");
    selfSender_ = new cMessage("selfSender");
    localPort_ = par("localPort");
    destPort_ = par("destPort");
    silences_ = par("silences");

    totalSentBytes_ = 0;
    warmUpPer_ = getSimulation()->getWarmupPeriod();
    voIPGeneratedThroughtput_ = registerSignal("voIPGeneratedThroughput");

    initTraffic_ = new cMessage("initTraffic");
    initTraffic();

    //Mobility information initialization
    ue = this->getParentModule();
//    getParentModule()->getS
    cModule *temp = getParentModule()->getSubmodule("mobility");
    if(temp != NULL){
        mobility = check_and_cast<inet::IMobility*>(temp);
    }
    else {
        EV << "UEWarningAlertApp::initialize - \tWARNING: Mobility module NOT FOUND!" << endl;
        throw cRuntimeError("UEWarningAlertApp::initialize - \tWARNING: Mobility module NOT FOUND!");
    }
    coordVal = 0;

    std::stringstream ss(evilVehicleID);
    while (ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        evilVehicles.insert(substr);
    }

    std::stringstream ss2(rsuIDs);
    while (ss2.good()) {
        std::string substr;
        getline(ss2, substr, ',');
        rsuSet.insert(substr);
    }

}

void VoIPSender::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (!strcmp(msg->getName(), "selfSender"))
            sendVoIPPacket();
        else if (!strcmp(msg->getName(), "selfSource"))
            selectPeriodTime();
        else
            initTraffic();
    }
}

void VoIPSender::initTraffic()
{
    std::string destAddress = par("destAddress").stringValue();
    cModule* destModule = getModuleByPath(par("destAddress").stringValue());
    if (destModule == nullptr)
    {
        // this might happen when users are created dynamically
        EV << simTime() << "VoIPSender::initTraffic - destination " << destAddress << " not found" << endl;

        simtime_t offset = 0.01; // TODO check value
        scheduleAt(simTime()+offset, initTraffic_);
        EV << simTime() << "VoIPSender::initTraffic - the node will retry to initialize traffic in " << offset << " seconds " << endl;
    }
    else
    {
        delete initTraffic_;

        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort_);

        int tos = par("tos");
        if (tos != -1)
            socket.setTos(tos);

        EV << simTime() << "VoIPSender::initialize - binding to port: local:" << localPort_ << " , dest: " << destAddress_.str() << ":" << destPort_ << endl;

        // calculating traffic starting time
        simtime_t startTime = par("startTime");

        scheduleAt(simTime()+startTime, selfSource_);
        EV << "\t starting traffic in " << startTime << " seconds " << endl;
    }
}

void VoIPSender::talkspurt(simtime_t dur)
{
    iDtalk_++;
    nframes_ = (ceil(dur / sampling_time));

    // a talkspurt must be at least 1 frame long
    if (nframes_ == 0)
        nframes_ = 1;

    EV << "VoIPSender::talkspurt - TALKSPURT[" << iDtalk_-1 << "] - Will be created[" << nframes_ << "] frames\n\n";

    iDframe_ = 0;
    nframesTmp_ = nframes_;
    scheduleAt(simTime(), selfSender_);
}

void VoIPSender::selectPeriodTime()
{
    if (!isTalk_)
    {
        double durSil2;
        if(silences_)
        {
            durSil_ = weibull(scaleSil_, shapeSil_);
            durSil2 = round(SIMTIME_DBL(durSil_)*1000) / 1000;
        }
        else
        {
            durSil_ = durSil2 = 0;
        }

        EV << "VoIPSender::selectPeriodTime - Silence Period: " << "Duration[" << durSil_ << "/" << durSil2 << "] seconds\n";
        scheduleAt(simTime() + durSil_, selfSource_);
        isTalk_ = true;
    }
    else
    {
        durTalk_ = weibull(scaleTalk_, shapeTalk_);
        double durTalk2 = round(SIMTIME_DBL(durTalk_)*1000) / 1000;
        EV << "VoIPSender::selectPeriodTime - Talkspurt[" << iDtalk_ << "] - Duration[" << durTalk_ << "/" << durTalk2 << "] seconds\n";
        talkspurt(durTalk_);
        scheduleAt(simTime() + durTalk_, selfSource_);
        isTalk_ = false;
    }
}

static bool isRSU(std::string id, std::unordered_set<std::string> rsuSet) {
    return (rsuSet.find(id) == rsuSet.end())?false:true;
}

//Choose the closest RSU to send the VoIP packet to while moving
std::string getDestination(std::unordered_set<std::string> rsuSet, std::string carID, std::unordered_map<std::string, veins::VeinsInetMobility *> &mobilityMap) {
    std::string closestRSU;
    double minDist = DBL_MAX;
    for(const auto& itr: rsuSet) {
        double dist = mobilityMap[itr]->getCurrentPosition().sqrdist(mobilityMap[carID]->getCurrentPosition());
        if(dist < minDist) {
            minDist = dist;
            closestRSU = itr;
        }
    }

    return closestRSU;
}

TrustManager *getRSUTrustManager(std::list<TrustManager*> &trustList, std::string nodeId) {
    auto tmpItr = trustList.begin();
    while(tmpItr != trustList.end()) {
        if((*tmpItr)->id.compare(nodeId) == 0) {
            return *tmpItr;
        }
        tmpItr++;
    }
    return nullptr;
}

void VoIPSender::sendVoIPPacket()
{
    cModule *tmp = getParentModule()->getSubmodule("mobility");
    veins::VeinsInetMobility *mobilityT = check_and_cast<veins::VeinsInetMobility *>(tmp);
    auto availableCars = mobilityT->getManager()->getManagedHosts();
    for(auto it = availableCars.begin(); it != availableCars.end(); it++) {
        std::string carId = it->second->getFullName();
        if(this->mobilityMap.find(carId) == this->mobilityMap.end()) {
            auto carMobility = check_and_cast<veins::VeinsInetMobility *>(it->second->getSubmodule("mobility"));
            this->mobilityMap[carId] = carMobility;
        }
    }
//
//    cout << "==================================\nPrinting submodules:" << endl;
//    for (cModule::SubmoduleIterator it(tmp); !it.end(); ++it) {
//        cout << "Module name: " << *(it) << endl;
//    }
//    cout << "==================================" << endl;

    if (destAddress_.isUnspecified())
        destAddress_ = L3AddressResolver().resolve(par("destAddress"));

    std::string senderID = this->ue->getFullName();
    omnetpp::cpp_string pktContent;
    MemoryOutputStream stream;

    //send trust data if i am not RSU
    if(!isRSU(senderID, this->rsuSet)) {
        TrustData content;

        Coord event_loc(0, 0, 0);

        //Generate an event location which is randomly far away in the
        //car's "sensor's reachability"
        Coord curr_loc = this->mobility->getCurrentPosition();
        auto crng = getEnvir()->getRNG(0);
        auto uniformDistVar = (int)omnetpp::uniform(crng, -10, 10);
        Coord newEvLoc(curr_loc.x + uniformDistVar, curr_loc.y + uniformDistVar, curr_loc.z + uniformDistVar);

        //simulate malicious vehicle advertising incorrect current velocity
        if(evilVehicles.find(senderID) != evilVehicles.end()) {
            // Coord evilVehicleLoc = this->randomLocGenerator();
            Coord evilVehicleLoc = Coord(10000, 10000, 10000);
            content = TrustData(simTime(), this->mobility->getCurrentPosition(), 
                        evilVehicleLoc, Coord(10000, 10000, 10000), senderID);
        }
        else {
            content = TrustData(simTime(), this->mobility->getCurrentPosition(), 
                        newEvLoc, this->mobility->getCurrentVelocity(), senderID);        
        }

        content.serializeTrustData(stream);

        std::string dest = getDestination(this->rsuSet, senderID, this->mobilityMap);
        destAddress_ = L3AddressResolver().resolve(dest.c_str());
    }
    //If I am RSU, send reputation lists, for now hardcoded to send to a specific receiving RSU
    else {
        if(!senderID.compare("car[0]")) {
            destAddress_ = L3AddressResolver().resolve("car[1]");
            cModule *app0Module = this->getParentModule()->getSubmodule("app[0]");
            //Note: the app[0] is as configured(VoIPReceiver) in Multiple-RSUs config in omnetpp.ini(simulations/NR/cars)
//            auto voipReceiverModule = mobilityMap[senderID]->getSubmodule("app[0]");
            auto voipReceiverModule = this->getParentModule()->getModuleByPath("Highway.car[0].app[0]");
            // auto voipReceiverModule = getModuleByPath("Scenario.car[0]")->getSubmodule("app[0]");
           VoIPReceiver *voipreceiverModule1 = check_and_cast<VoIPReceiver *>(voipReceiverModule);

            //get reputation list of car[0] from the voipreceiver module
            // MemoryOutputStream stream;
           TrustManager *rsuManager = getRSUTrustManager(voipreceiverModule1->trustListAllVehicles, senderID);
           if(rsuManager == nullptr) {
               return;
           }
           serializeReputationList(stream, rsuManager);
        }
        else if(!senderID.compare("car[1]")) {
            destAddress_ = L3AddressResolver().resolve("car[0]");

            //Note: the app[2] is as configured(VoIPReceiver) in Multiple-RSUs config in omnetpp.ini(simulations/NR/cars)
//            auto voipreceiverModule = mobilityMap[senderID]->getSubmodule("app[2]");
            auto voipReceiverModule = this->getParentModule()->getModuleByPath("Highway.car[1].app[0]");
           VoIPReceiver *voipreceiverModule1 = check_and_cast<VoIPReceiver *>(voipReceiverModule);
//
//            //get reputation list of car[1] from the voipreceiver module
           TrustManager *rsuManager = getRSUTrustManager(voipreceiverModule1->trustListAllVehicles, senderID);
           if(rsuManager == nullptr) {
               return;
           }
           serializeReputationList(stream, rsuManager);
        }
    }
    std::vector<uint8_t> serialized_data;
    stream.copyData(serialized_data);
    for(size_t i = 0; i < serialized_data.size(); i++) {
        pktContent += serialized_data[i];
    }

    omnetpp::cpp_string sender;
    sender += senderID;

    Packet* packet = new inet::Packet("VoIP");
    auto voip = makeShared<VoipPacket>();
    voip->setIDtalk(iDtalk_ - 1);
    voip->setNframes(nframes_);
    voip->setIDframe(iDframe_);
    voip->setPayloadTimestamp(simTime());
    voip->setChunkLength(B(size_));
    voip->setSender(sender);
    voip->setSerializedMessage(pktContent);
    voip->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(voip);
    EV << "VoIPSender::sendVoIPPacket - Talkspurt[" << iDtalk_-1 << "] - Sending frame[" << iDframe_ << "]\n";

    socket.sendTo(packet, destAddress_, destPort_);
    --nframesTmp_;
    ++iDframe_;

    // emit throughput sample
    totalSentBytes_ += size_;
    double interval = SIMTIME_DBL(simTime() - warmUpPer_);
    if (interval > 0.0)
    {
        double tputSample = (double)totalSentBytes_ / interval;
        emit(voIPGeneratedThroughtput_, tputSample );
    }

    if (nframesTmp_ > 0)
        scheduleAt(simTime() + sampling_time, selfSender_);
}
