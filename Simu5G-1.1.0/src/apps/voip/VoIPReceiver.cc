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

#include "apps/voip/VoIPReceiver.h"
#include <string>
#include <unordered_set>
#include "inet/common/geometry/common/Coord.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

std::string rsuID = "car[0],car[1]";

Define_Module(VoIPReceiver);

using namespace std;
using namespace inet;

VoIPReceiver::~VoIPReceiver()
{
    while (!mPlayoutQueue_.empty())
    {
        delete mPlayoutQueue_.front();
        mPlayoutQueue_.pop_front();
    }

    while (!mPacketsList_.empty())
    {
        delete mPacketsList_.front();
        mPacketsList_.pop_front();
    }
}

void VoIPReceiver::initialize(int stage)
{
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    emodel_Ie_ = par("emodel_Ie_");
    emodel_Bpl_ = par("emodel_Bpl_");
    emodel_A_ = par("emodel_A_");
    emodel_Ro_ = par("emodel_Ro_");

    mBufferSpace_ = par("dim_buffer");
    mSamplingDelta_ = par("sampling_time");
    mPlayoutDelay_ = par("playout_delay");

    mInit_ = true;

    L3Address addr = L3AddressResolver().addressOf(getParentModule());
    int port = par("localPort");

    cout << "[TRUST_MANAGEMENT] VoIPReceiver::initialize - binding to address/port:" << addr.str() << ":" <<
            port << " Running on: " << getParentModule()->getFullName() << endl;

    EV << "VoIPReceiver::initialize - binding to port: local:" << port << endl;
    if (port != -1)
    {
        socket.setOutputGate(gate("socketOut"));
        socket.bind(port);
    }

    totalRcvdBytes_ = 0;
    warmUpPer_ = getSimulation()->getWarmupPeriod();

    voIPFrameLossSignal_ = registerSignal("voIPFrameLoss");
    voIPFrameDelaySignal_ = registerSignal("voIPFrameDelay");
    voIPPlayoutDelaySignal_ = registerSignal("voIPPlayoutDelay");
    voIPMosSignal_ = registerSignal("voIPMos");
    voIPTaildropLossSignal_ = registerSignal("voIPTaildropLoss");
    voIPJitterSignal_ = registerSignal("voIPJitter");
    voIPPlayoutLossSignal_ = registerSignal("voIPPlayoutLoss");
    voIPReceivedThroughput_ = registerSignal("voIPReceivedThroughput");

    alertSendMsgSignal_ = registerSignal("alertSendMsg");
    simtime_t startTime = par("startTime");

    numRecvd = 0;
    WATCH(numRecvd);
    // doingRemoteAttestation = false

    trustListAllVehicles = new TrustManager();
    WATCH_VECTOR(trustListAllVehicles->trustNodes);
}

/**
 * @brief Weighted averaging(reputations of cars are weights) based event location estimator
 * 
 * @param messagesReceived map of already fully trusted cars and the most recent messages sent by them
 * @param rsuManager       Reputation lists manager at the RSU
 * @param msg              Currently received message
 * @return inet::Coord     Estimated event location
 */
inet::Coord weightedAveragingestimateEventLocation(std::unordered_map<std::string, TrustData *> &messagesReceived, 
                                    TrustManager *rsuManager, TrustData *msg) {
    if(messagesReceived.empty()) {
        return msg->getEventLocation();
    }

    inet::Coord num = Coord(0,0,0);
    double deno = 0.0;
    for(auto message: messagesReceived) {
        auto trustNode = rsuManager->getTrustNode(message.first);
        if(trustNode != nullptr) {
            Coord newCoord = message.second->getEventLocation() * trustNode->reputation;
            num += newCoord;
            deno += trustNode->reputation;
        }
    }
    // Not possible but just doing to avoid any transient errors in the simulation
    if(deno == 0) {
        return msg->getEventLocation();
    }

    return num/deno;
}

/**
 * @brief Normal averaging based event location estimator
 * 
 * @param messagesReceived map of already fully trusted cars and the most recent messages sent by them
 * @param rsuManager       Reputation lists manager at the RSU
 * @param msg              Currently received message
 * @return inet::Coord     Estimated event location
 */
inet::Coord averagingBasedEstimateEventLocation(std::unordered_map<std::string, TrustData *> &messagesReceived, 
                                    TrustManager *rsuManager, TrustData *msg) {
    if (messagesReceived.empty()) {
        return msg->getEventLocation();
    }

    inet::Coord num = Coord(0,0,0);
    int deno = 0;
    for (auto message: messagesReceived) {
        auto trustNode = rsuManager->getTrustNode(message.first);
        if(trustNode != nullptr) {
            Coord newCoord = message.second->getEventLocation();
            num += newCoord;
            deno++;
        }
    }
    // Not possible but just doing to avoid any transient errors in the simulation
    if (deno == 0) {
        return msg->getEventLocation();
    }

    return num/deno;
}

/**
 * @brief K-means clustering based event location estimator
 * 
 * @param messagesReceived map of already fully trusted cars and the most recent messages sent by them
 * @param rsuManager       Reputation lists manager at the RSU
 * @param msg              Currently received message
 * @return inet::Coord     Estimated event location(centroid of the largest cluster)
 */
inet::Coord clusteringEventLocationEstimator(std::unordered_map<std::string, TrustData *> &messagesReceived, 
                                    TrustManager *rsuManager, TrustData *msg) {
    if(rsuManager->trustNodes.empty()) {
        return msg->getEventLocation();
    }

    vector<Point> points;
    for(auto message: messagesReceived) {
        Coord c = message.second->getEventLocation();
        points.push_back(Point(c.x, c.y));
    }
    vector<Point> centroids = kMeansClustering(&points, 10, 2);
    int i = 0;
    Point majorityCentroid;
    for(auto centroid: centroids) {
        cout << "Centroid[" << i++ << "]: "<< centroid.nPoints <<", (" << 
            centroid.x << ", " << centroid.y << ")" << endl;
        if(majorityCentroid.nPoints < centroid.nPoints) {
            majorityCentroid = centroid;
        }
    }

    return Coord(majorityCentroid.x, majorityCentroid.y);
}

/**
 * @brief Utility function to print reputations at a particular time in the simulation
 *        for analysis later
 * 
 * @param rsuManager    Reputation lists manager at the RSU
 * @param time          Simulation time at which the reputations have to be printed
 */
void print_reputations(TrustManager *rsuManager, simtime_t time) {
    std::string line(time.str());
    for(auto trustNode: rsuManager->trustNodes) {
        line += (", " + trustNode->nodeId + ", " + to_string(trustNode->reputation));
    }
    std::cout << "Reputations, " << line << std::endl;
}

/**
 * @brief Simulate remote attestation by simply setting a timer to send an event message(`remoteAttestationDoneMsg`)
 *        to VoIPReceiver indicating remote attestation is done and change the remote attestee's reputation to the 
 *        appropriate value
 * 
 *        Possible bug:
 *          - Cannot start "remote attestation" of multiple cars simultaneously
 * 
 * @param doingRemoteAttestation 
 * @param sender                    Remote Attestee
 */
void VoIPReceiver::doRemoteAttestation(std::string sender) {
    auto senderTrustNode = this->trustListAllVehicles->getTrustNode(sender);
    RemoteAttestationMsg *remoteAttestationDoneMsg = new RemoteAttestationMsg("remoteAttestationDoneMsg");
    if(!remoteAttestationDoneMsg->isScheduled()) {
        remoteAttestationDoneMsg->setSender(sender.c_str());
        scheduleAt(simTime() + REMOTE_ATTESTATION_TIME, remoteAttestationDoneMsg);
        std::cout << "Start remote attestation at " << simTime() << std::endl;
    }
}

/**
 * @brief Update list of trust cars based on the recently assigned reputation to the car
 *        based on the description in the document(https://www.overleaf.com/read/nwjssqvbcmtc)
 * 
 * @param messagesReceived     Hashmap of messages from fully trusted cars
 * @param estimateLocation     Estimate event location
 * @param msgData              New message to be added to the map
 * @param reputation           Reputation assigned to the sender based on this new message calculated by the trust
 *                             management function
 */
void updateTrustSet(std::unordered_map<std::string, TrustData *> &messagesReceived, inet::Coord estimateLocation,
                        TrustData *msgData, double reputation) {
    double dist = msgData->getEventLocation().sqrdist(estimateLocation);
    if(messagesReceived.size() >= MIN_NODES_FOR_REPUTATION_UPDATE) {
//        if(reputation < TRUST_THRESHOLD || dist >= ERROR_MARGIN) {
        if(reputation < TRUST_THRESHOLD) {



        //  std::cout << "====================" << std::endl;
        //  std::cout << "Faulty "<< msgData->getSenderID() <<" => Removing its messages" << std::endl;
            if(messagesReceived.find(msgData->getSenderID()) != messagesReceived.end()) {
        //      std::cout << "Found some previous message for this faulty car" << std::endl;
                messagesReceived.erase(msgData->getSenderID());
            }
        //  std::cout << "====================" << std::endl;
        } else {
            auto itr = messagesReceived.find(msgData->getSenderID());
            if(itr == messagesReceived.end()) {
                //Message from this sender already in map, just update with new message
                messagesReceived[msgData->getSenderID()] = msgData;
            }
            else {
                //Message from this sender not in map, add new message
                auto toBeDeleted = itr->second;
                itr->second = msgData;
                delete toBeDeleted;
            }

        }
    }
//    else if(reputation >= TRUST_THRESHOLD && dist < ERROR_MARGIN) {
    else {
        //Add message to the latest messages received map
        auto itr = messagesReceived.find(msgData->getSenderID());
        if(itr == messagesReceived.end()) {
            //Message from this sender already in map, just update with new message
            messagesReceived[msgData->getSenderID()] = msgData;
        }
        else {
            //Message from this sender not in map, add new message
            auto toBeDeleted = itr->second;
            itr->second = msgData;
            delete toBeDeleted;
        }
    }
}

/*
 * @brief Trust Management function
 * This function updates the reputation of the car by comparing the estimate location based on the algorithm 
 * described in the document(https://www.overleaf.com/read/nwjssqvbcmtc)
 * 
 * @param messagesReceived  list of latest (fully trusted)message received from cars at RSU  
 * @param msg               Message based on which trust has to be evaluated
 * @param rsuID             rsu id
 * @param recvr_class       class object of VoIPReceiver to be used by remote attestor 
 * @return a boolean denoting if the sender's message is trusted enough to be forwarded
 */
bool VoIPReceiver::doTrustManagement(std::unordered_map<std::string, TrustData *> &messagesReceived, TrustData *msg, 
                                        std::string rsuID, VoIPReceiver *recvr_class) {
    // simtime_t startTime = simTime();
    string sender(msg->getSenderID());
    // cout << "[TRUST_MANAGEMENT_INFO] Sender is: " << sender << endl;
    bool messageReceivedByRSU = false;

    //Drop message if reputation of sender is lower than threshold
    if(sender.compare(rsuID) != 0) {
        TrustNodeList *nodeTrust = this->trustListAllVehicles->getTrustNode(sender);
        if(nodeTrust != nullptr && nodeTrust->reputation < TRUST_THRESHOLD) {
            return false;
        }
    }

    TrustNodeList *trustNode = this->trustListAllVehicles->getTrustNode(sender);
    auto time = simTime();
    // auto estimatedEventLocation = averagingBasedEstimateEventLocation(messagesReceived, this->trustListAllVehicles, msg);
    auto estimatedEventLocation = weightedAveragingestimateEventLocation(messagesReceived, this->trustListAllVehicles, msg);
    // auto estimatedEventLocation = clusteringEventLocationEstimator(messagesReceived, this->trustListAllVehicles, msg);
    auto msg_ev_loc = msg->getEventLocation();

    // cout << "ReceivedEventLocation: " << msg->getSenderID() << ", " << time << ", " << msg_ev_loc << endl;
    // cout << "EstimateLocation: " << time << ", " << estimatedEventLocation << endl;

    // Distance between estimated event location and msg event location
    // cout << msg->getSenderID() << ": Estimated Location - " << estimatedEventLocation << endl;
    double dist = msg_ev_loc.sqrdist(estimatedEventLocation);

//    std::cout << "Message Location: " << msg_ev_loc.str() << std::endl;
//    std::cout << "Estimated Location: " << estimatedEventLocation.str() << std::endl;
//    std::cout << "Distance: " << dist << std::endl;

    if (trustNode == nullptr) {
        this->trustListAllVehicles->addEntryTrustMap(sender, DEFAULT_TRUST);
    }
    #ifdef ALGO_3_2
    else if(messagesReceived.size() >= MIN_NODES_FOR_REPUTATION_UPDATE) {
    #else
    else {
    #endif
        if(dist > ERROR_MARGIN) {
//            std::cout << "-----------------------------------" << std::endl;
//            std::cout << "Sender " << sender << " reputation is below the threshold:" << std::endl;
//            std::cout << "Message Location: " << msg_ev_loc.str() << std::endl;
//            std::cout << "Estimated Location: " << estimatedEventLocation.str() << std::endl;
//            std::cout << "Distance: " << dist << std::endl;

            // Checks if each coordinate of the event location in the message is within ERROR_MARGIN
            // of the estimated event location and penalizing the node if it isn't
            //Only modify if sender(sensor of event) is not RSU
            if(sender.compare(rsuID)) {
                // Calculate reduction factor 
                // double reductionFactor = (1.0 - ((double)dist/INT32_MAX));
                // double reductionFactor = 0.9;
                double reductionFactor = (1.0 - ((double)dist/(2 * MAP_RANGE)));
//                std::cout << "Previous rept: " << trustNode->reputation << " Reduction factor: " << reductionFactor << " Next rept: " << max(0.0, trustNode->reputation * reductionFactor) << std::endl;
                trustNode->reputation = max(0.0, trustNode->reputation * reductionFactor);
            }
//            std::cout << "-----------------------------------" << std::endl;
            #ifdef PROBATION
            print_reputations(this->trustListAllVehicles, time);
            return false;
            #endif
        }
        else if (dist <= ERROR_MARGIN) {
            //The RSU has highest reputation by default and no need to calculate the reputation of the RSU
            //again
            if(sender.compare(rsuID)) {
                //Don't let reputation go above 1.0
                trustNode->reputation = min(trustNode->reputation * DIRECT_REPUTATION_REWARD, 1.0);
            }
        }
    }

    //Printing to get data for graphs
    print_reputations(this->trustListAllVehicles, time);
    trustNode = this->trustListAllVehicles->getTrustNode(sender);
    updateTrustSet(messagesReceived, estimatedEventLocation, msg, trustNode->reputation);

//    std::cout << "_________________________" << std::endl;
//    std::cout << "New Trust List Members" << std::endl;
//    for (auto const& pair : messagesReceived) {
//            std::cout << pair.first << ' ';
//    }
//    std::cout << std::endl;
//    std::cout << "_________________________" << std::endl;
    if(trustNode->reputation > TRUST_THRESHOLD) {
        return true;
    }
    else {
        //Perform remote attestation as and when needed is reputation of node goes down a threshold
        // doRemoteAttestation(sender, rsuManager);
//        cout << "KickedOut: " << msg->getSenderID() << ", " << time << endl;
        return false;
    }
}

static bool isRSU(std::string id, std::unordered_set<std::string> rsuSet) {
    return (rsuSet.find(id) == rsuSet.end())?false:true;
}

void VoIPReceiver::handleMessage(cMessage *msg)
{
    bool trusted = true;
    //Using the name of the vehicle as the identifier instead of the IP
    std::string selfID = getParentModule()->getFullName();
    // cout << "[TRUST_MANAGEMENT] Got a packet" << endl;


    if (msg->isSelfMessage()) {
        // Process the remote attestation done message and update the reputation of the remote attestee
        if(!strcmp(msg->getName(), "remoteAttestationDoneMsg")) {
            // doingRemoteAttestation = false;
            RemoteAttestationMsg *remoteAttestationMsg = check_and_cast<RemoteAttestationMsg *>(msg);
            std::string sender(remoteAttestationMsg->getSender());
            auto senderTrustNode = this->trustListAllVehicles->getTrustNode(sender);
            senderTrustNode->reputation = DEFAULT_TRUST;
            std::cout << "Remote attestation done at " << simTime()<< std::endl;
            delete remoteAttestationMsg;
        }
        return;
    }
    numRecvd++;

    Packet* pPacket = check_and_cast<Packet*>(msg);

    if (pPacket == 0)
    {
        throw cRuntimeError("VoIPReceiver::handleMessage - FATAL! Error when casting to inet packet");
    }

    // read VoIP header
    auto voipHeader = pPacket->popAtFront<VoipPacket>();

    auto hdr = voipHeader->dup();

    //Reconstruct msg data from the message
    TrustData *trustMsgContent = new TrustData();
    const uint8_t *serializedTrustData = (const unsigned char *)(hdr->getSerializedMessage().c_str());
    MemoryInputStream stream(serializedTrustData, B(hdr->getSerializedMessage().length()));
    trustMsgContent->deserializeTrustData(stream);


    //Do trust management
    if(!doTrustManagement(messagesReceived, trustMsgContent, selfID, this)) {
        trusted = false;
    }

    if (mInit_)
    {
        mCurrentTalkspurt_ = voipHeader->getIDtalk();
        mInit_ = false;
    }

    if (mCurrentTalkspurt_ != voipHeader->getIDtalk())
    {
        playout(false);
        mCurrentTalkspurt_ = voipHeader->getIDtalk();
    }

    EV << "VoIPReceiver::handleMessage - Packet received: TALK[" << voipHeader->getIDtalk() << "] - FRAME[" << voipHeader->getIDframe() << " size: " << voipHeader->getChunkLength() << " bytes]\n";

    // emit throughput sample
    totalRcvdBytes_ += (int)voipHeader->getChunkLength().get();
    double interval = SIMTIME_DBL(simTime() - warmUpPer_);
    if (interval > 0.0)
    {
        double tputSample = (double)totalRcvdBytes_ / interval;
        emit(voIPReceivedThroughput_, tputSample );
    }


    auto packetToBeQueued = voipHeader->dup();
    packetToBeQueued->setArrivalTime(simTime());
    mPacketsList_.push_back(packetToBeQueued);

    if(trusted) {
        /* Sending signal to "forward" the alert to everyone in the network */
        emit(alertSendMsgSignal_, pPacket);
    }

    delete pPacket;
}

void VoIPReceiver::playout(bool finish)
{
    if (mPacketsList_.empty())
        return;

    double sample;

    VoipPacket* pPacket = mPacketsList_.front();

    simtime_t firstPlayoutTime = pPacket->getArrivalTime() + mPlayoutDelay_;
    unsigned int n_frames = pPacket->getNframes();
    unsigned int playoutLoss = 0;
    unsigned int tailDropLoss = 0;
    unsigned int channelLoss;

    if (finish)
    {
        PacketsList::iterator it;
        unsigned int maxId = 0;
        for (it = mPacketsList_.begin(); it != mPacketsList_.end(); it++)
            maxId = std::max(maxId, (*it)->getIDframe());
        channelLoss = maxId + 1 - mPacketsList_.size();
    }

    else
        channelLoss = pPacket->getNframes() - mPacketsList_.size();

    sample = ((double) channelLoss / (double) n_frames);
    emit(voIPFrameLossSignal_, sample);

    //Vector for managing duplicates
    std::vector<bool> isArrived;
    isArrived.resize(n_frames, false);

    simtime_t last_jitter = 0.0;
    simtime_t max_jitter = -1000.0;

    while (!mPacketsList_.empty())
    {
        pPacket = mPacketsList_.front();

        sample = SIMTIME_DBL(pPacket->getArrivalTime() - pPacket->getPayloadTimestamp());
        emit(voIPFrameDelaySignal_, sample);

        unsigned int IDframe = pPacket->getIDframe();

        pPacket->setPlayoutTime(firstPlayoutTime + IDframe * mSamplingDelta_);

        last_jitter = pPacket->getArrivalTime() - pPacket->getPlayoutTime();
        max_jitter = std::max(max_jitter, last_jitter);

        // avoid printing during finish (as it will print to the standard output)
        if(!finish)
            EV << "VoIPReceiver::playout - Jitter measured: " << last_jitter << " TALK[" << pPacket->getIDtalk() << "] - FRAME[" << IDframe << "]\n";

        if (IDframe < n_frames)
        {
            //Duplicates management
            if (isArrived[IDframe])
            {
                // avoid printing during finish (as it will print to the standard output)
                if(!finish)
                    EV << "VoIPReceiver::playout - Duplicated Packet: TALK[" << pPacket->getIDtalk() << "] - FRAME[" << IDframe << "]\n";
                delete pPacket;
            }
            else if( last_jitter > 0.0 )
            {
                ++playoutLoss;
                // avoid printing during finish (as it will print to the standard output)
                if(!finish)
                    EV << "VoIPReceiver::playout - out of time packet deleted: TALK[" << pPacket->getIDtalk() << "] - FRAME[" << IDframe << "]\n";
                emit(voIPJitterSignal_, last_jitter);
                delete pPacket;
            }
            else
            {
                while( !mPlayoutQueue_.empty() && pPacket->getArrivalTime() > mPlayoutQueue_.front()->getPlayoutTime() )
                {
                    ++mBufferSpace_;
                    delete mPlayoutQueue_.front();
                    mPlayoutQueue_.pop_front();
                }

                if(mBufferSpace_ > 0)
                {
                    // avoid printing during finish (as it will print to the standard output)
                    if(!finish)
                    {
                        EV << "VoIPReceiver::playout - Sampleable packet inserted into buffer: TALK["<< pPacket->getIDtalk() << "] - FRAME[" << IDframe
                           << "] - arrival time[" << pPacket->getArrivalTime() << "] -  sampling time[" << pPacket->getPlayoutTime() << "]\n";
                    }
                    --mBufferSpace_;

                    //duplicates management
                    isArrived[IDframe] = true;

                    mPlayoutQueue_.push_back(pPacket);
                }
                else
                {
                    ++tailDropLoss;
                    // avoid printing during finish (as it will print to the standard output)
                    if(!finish)
                    {
                        EV << "VoIPReceiver::playout - Buffer is full, discarding packet: TALK[" << pPacket->getIDtalk() << "] - FRAME["
                           << IDframe << "] - arrival time[" << pPacket->getArrivalTime() << "]\n";
                    }
                    delete pPacket;
                }
            }
        }

        mPacketsList_.pop_front();
    }

    double proportionalLoss = ((double) tailDropLoss + (double) playoutLoss + (double) channelLoss) / (double) n_frames;
    // avoid printing during finish (as it will print to the standard output)
    if(!finish)
    {
        EV << "VoIPReceiver::playout - proportionalLoss " << proportionalLoss << "(tailDropLoss=" << tailDropLoss << " - playoutLoss="
           <<  playoutLoss << " - channelLoss=" << channelLoss << ")\n\n";
    }

    double mos = eModel(mPlayoutDelay_, proportionalLoss);
    emit(voIPPlayoutDelaySignal_, mPlayoutDelay_);

    sample = ((double) playoutLoss / (double) n_frames);
    emit(voIPPlayoutLossSignal_, sample);

    sample = mos;
    emit(voIPMosSignal_, sample);

    sample = ((double) tailDropLoss / (double) n_frames);
    emit(voIPTaildropLossSignal_, sample);

    // avoid printing during finish (as it will print to the standard output)
    if(!finish)
    {
        EV << "VoIPReceiver::playout - Computed MOS: eModel( " << mPlayoutDelay_ << " , " << tailDropLoss << "+" << playoutLoss << "+"
           << channelLoss << " ) = " << mos << "\n";

        EV << "VoIPReceiver::playout - Playout Delay Adaptation \n" << "\t Old Playout Delay: " << mPlayoutDelay_ << "\n\t Max Jitter Measured: "
           << max_jitter << "\n\n";
    }

    mPlayoutDelay_ += max_jitter;
    if (mPlayoutDelay_ < 0.0)
        mPlayoutDelay_ = 0.0;
    // avoid printing during finish (as it will print to the standard output)
    if(!finish)
        EV << "\t New Playout Delay: " << mPlayoutDelay_ << "\n\n";
}

double VoIPReceiver::eModel(simtime_t delay, double loss)
{
    double delayms = 1000 * SIMTIME_DBL(delay);

    // Compute the Id parameter
    int u = ((delayms - 177.3) > 0 ? 1 : 0);
    double id = 0.0;
    id = 0.024 * delayms + 0.11 * (delayms - 177.3) * u;

    // Packet loss p in %
    double p = loss * 100;
    // Compute the Ie,eff parameter
    double ie_eff = emodel_Ie_ + (95 - emodel_Ie_) * p / (p + emodel_Bpl_);

    // Compute the R factor
    double Rfactor = emodel_Ro_ - id - ie_eff + emodel_A_;

    // Compute the MOS value
    double mos = 0.0;

    if (Rfactor < 0)
    {
        mos = 1.0;
    }
    else if (Rfactor > 100)
    {
        mos = 4.5;
    }
    else
    {
        mos = 1 + 0.035 * Rfactor + 7 * pow(10, (double) -6) * Rfactor *
            (Rfactor - 60) * (100 - Rfactor);
    }

    mos = (mos < 1) ? 1 : mos;

    return mos;
}

void VoIPReceiver::finish()
{
    // last talkspurt playout
    playout(true);
}

/**
 * Perform k-means clustering
 * @param points - pointer to vector of points
 * @param epochs - number of k means iterations
 * @param k - the number of initial centroids
 */
vector<Point> kMeansClustering(vector<Point>* points, int epochs, int k) {
    int n = points->size();

    // Randomly initialise centroids
    // The index of the centroid within the centroids vector
    // represents the cluster label.
    vector<Point> centroids;
    srand(time(0));
    for (int i = 0; i < k; ++i) {
        centroids.push_back(points->at(rand() % n));
    }

    for (int i = 0; i < epochs; ++i) {
        // For each centroid, compute distance from centroid to each point
        // and update point's cluster if necessary
        for (vector<Point>::iterator c = begin(centroids); c != end(centroids);
             ++c) {
            int clusterId = c - begin(centroids);

            for (vector<Point>::iterator it = points->begin();
                 it != points->end(); ++it) {
                Point p = *it;
                double dist = c->distance(p);
                if (dist < p.minDist) {
                    p.minDist = dist;
                    p.cluster = clusterId;
                }
                *it = p;
            }
        }

        // Create vectors to keep track of data needed to compute means
        vector<int> nPoints;
        vector<double> sumX, sumY;
        for (int j = 0; j < k; ++j) {
            nPoints.push_back(0);
            sumX.push_back(0.0);
            sumY.push_back(0.0);
        }

        // Iterate over points to append data to centroids
        for (vector<Point>::iterator it = points->begin(); it != points->end();
             ++it) {
            int clusterId = it->cluster;
            nPoints[clusterId] += 1;
            sumX[clusterId] += it->x;
            sumY[clusterId] += it->y;

            it->minDist = __DBL_MAX__;  // reset distance
        }
        // Compute the new centroids
        for (vector<Point>::iterator c = begin(centroids); c != end(centroids);
             ++c) {
            int clusterId = c - begin(centroids);
            c->x = sumX[clusterId] / nPoints[clusterId];
            c->y = sumY[clusterId] / nPoints[clusterId];
            c->nPoints = nPoints[clusterId];
        }
    }
    return centroids;
}

