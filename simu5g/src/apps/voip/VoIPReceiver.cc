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

#include "inet/common/geometry/common/Coord.h"

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

    int port = par("localPort");
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
    doingRemoteAttestation = false;
}

inet::Coord estimateEventLocation(std::unordered_map<std::string, TrustData *> &messagesReceived, TrustManager *rsuManager,
                                    TrustData *msg) {
    auto itr = rsuManager->head;
    if(!itr) {
        return msg->getEventLocation();
    }

    inet::Coord num = Coord(0,0,0);
    double deno = 0.0;
    while(itr != nullptr) {
        auto message = messagesReceived.find(itr->nodeId);
        Coord newCoord = message->second->getEventLocation() * itr->reputation;
        // num += Coord(itr->directReputation, itr->directReputation, itr->directReputation) * message->second->getEventLocation();
        num += newCoord;
        deno += itr->reputation;
        itr = itr->next;
    }
    return num/deno;
}

void print_reputations(TrustManager *rsuManager, simtime_t time) {
    std::string line(time.str());
    std::string nodesLine(time.str());
    auto tmp = rsuManager->head;
    while(tmp != nullptr) {
        line += (", " + to_string(tmp->reputation));
        nodesLine += (", " + tmp->nodeId);
        tmp = tmp->next;
    }
    std::cout << "Reputations, " << line << std::endl;
    std::cout << "Reputations, " << nodesLine << std::endl;
}

void VoIPReceiver::doRemoteAttestation(bool &doingRemoteAttestation, std::string sender, TrustManager* rsuTrustManager) {
    auto senderTrustNode = rsuTrustManager->getTrustNode(sender);
    RemoteAttestationMsg *remoteAttestationDoneMsg = new RemoteAttestationMsg("remoteAttestationDoneMsg");
    if(!remoteAttestationDoneMsg->isScheduled()) {
        doingRemoteAttestation=true;
        remoteAttestationDoneMsg->setSender(sender.c_str());
        scheduleAt(simTime() + REMOTE_ATTESTATION_TIME, remoteAttestationDoneMsg);
        std::cout << "Start remote attestation at " << simTime() << std::endl;
    }
}

bool VoIPReceiver::doTrustManagement(std::unordered_map<std::string, TrustData *> &messagesReceived, std::list<TrustManager *> &trustListAllVehicles,
                         TrustData *msg, std::string rsuID, VoIPReceiver *recvr_class) {
    // simtime_t startTime = simTime();
    string sender(msg->getSenderID());
    bool messageReceivedByRSU = false;

    //Get list of reputations stored at RSU
    TrustManager *rsuItr = findVehicleInList(trustListAllVehicles, rsuID);
    if(rsuItr == nullptr) {
        TrustManager *tmp = new TrustManager(rsuID);
        trustListAllVehicles.push_back(tmp);
        rsuItr = findVehicleInList(trustListAllVehicles, rsuID);
    }

    //Drop message if reputation of sender is lower than threshold
    if(sender.compare(rsuID) != 0) {
        TrustNodeList *nodeTrust = rsuItr->getTrustNode(sender);
        if(nodeTrust != nullptr && nodeTrust->reputation < TRUST_THRESHOLD) {
            return false;
        }
    }
    
    auto estimatedEventLocation = estimateEventLocation(messagesReceived, rsuItr, msg);

    //Checks if each coordinate of the event location in the message is within 0.001 error margin
    //of the estimated event location
    //and penalizing the node if it isn't
    if(estimatedEventLocation != msg->getEventLocation()) {
        //Calculate euclidean distance b/w expected ev location and msg ev location
        auto msg_ev_loc = msg->getEventLocation();

        //Distance between estimated event location and msg event location
        double dist = msg_ev_loc.sqrdist(estimatedEventLocation);

        TrustNodeList *trustNode = rsuItr->getTrustNode(sender);

        //reduce direct reputation proportionately
        if(trustNode != nullptr) {
            //Only modify if sender(sensor of event) is not RSU
            if(sender.compare(rsuID)) {
                dist = dist;
                //higher demotion by squaring and cubing the distance
                //Update: Does not work since the demotion happens too fast!
                // dist = dist * dist;
                // dist = dist * dist * dist;
                //Dividing distance by a very large number to get a ratio of how much to penalize
                double reductionFactor = (1.0 - ((double)dist/INT32_MAX));
                trustNode->reputation = max(0.0, trustNode->reputation * reductionFactor);
                //TODO: Add assertions here to check the direct reputation does not go down for good nodes
            }
        }
        else {
            //Add reputation of rsu in its own list as 1(highest reputation)
            if(!sender.compare(rsuID)) {
                rsuItr->addEntryTrustMap(sender, 1);
            }
            //Add default reputations for all other nodes
            else {
                rsuItr->addEntryTrustMap(sender, DEFAULT_TRUST);
            }
        }
    }
    else {
        TrustNodeList *trustNode = rsuItr->getTrustNode(sender);
        //No entry for this sender in list, create new entry with default reputations
        if(trustNode == nullptr) {
            //The RSU has highest reputation at all nodes by default
            if(!sender.compare(rsuID)) {
                rsuItr->addEntryTrustMap(sender, 1);
            }
            else {
                rsuItr->addEntryTrustMap(sender, DEFAULT_TRUST);
            }
        }
        else {
            //The RSU has highest reputation by default and no need to calculate the reputation of the RSU
            //again
            if(sender.compare(rsuID)) {
                //modify entry in trust map
                //Don't let reputation go above 1.0
                trustNode->reputation = min(trustNode->reputation * DIRECT_REPUTATION_REWARD, 1.0);
            }
        }
    }

    //Just return true if the message was from the rsu itself(of it seeing/directly experiencing the event)
    //cause recommended reputation calculation not needed for event messages generated by the rsu
    if(sender.compare(rsuID) == 0) {
        //Printing to get data for graphs
        std::cout << sender << ", 1, " << simTime() << ", " << recvr_class->numRecvd << std::endl;
        return true;
    }

    double msgSenderReputation = getReputation(rsuItr, sender);

    //Printing to get data for graphs
    simtime_t time = simTime();
    std::cout << sender << ", " << msgSenderReputation << ", " << time << ", " << recvr_class->numRecvd << std::endl;
    print_reputations(rsuItr, time);

    if(msgSenderReputation > TRUST_THRESHOLD) {
        return true;
    }
    else {
        //Perform remote attestation as and when needed is reputation of node goes down a threshold
        // doRemoteAttestation(recvr_class->doingRemoteAttestation, sender, rsuManager);
        return false;
    }
}

void VoIPReceiver::handleMessage(cMessage *msg)
{
    bool trusted = true;
    //Using the name of the vehicle as the identifier instead of the IP
    std::string selfID = getParentModule()->getFullName();

    if (msg->isSelfMessage()) {
        if(!strcmp(msg->getName(), "remoteAttestationDoneMsg")) {
            doingRemoteAttestation = false;
            RemoteAttestationMsg *remoteAttestationMsg = check_and_cast<RemoteAttestationMsg *>(msg);
            std::string sender(remoteAttestationMsg->getSender());
            auto rsuManager = findVehicleInList(trustListAllVehicles, selfID);
            auto senderTrustNode = rsuManager->getTrustNode(sender);
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
    if(!doTrustManagement(messagesReceived, trustListAllVehicles, trustMsgContent, selfID, this)) {
        trusted = false;
    }
    else {
        //Add message to the latest messages received map
        auto itr = messagesReceived.find(trustMsgContent->getSenderID());
        if(itr == messagesReceived.end()) {
            //Message from this sender already in map, just update with new message
            messagesReceived[trustMsgContent->getSenderID()] = trustMsgContent;
        }
        else {
            //Message from this sender not in map, add new message
            auto toBeDeleted = itr->second;
            itr->second = trustMsgContent;
            delete toBeDeleted;
        }
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

