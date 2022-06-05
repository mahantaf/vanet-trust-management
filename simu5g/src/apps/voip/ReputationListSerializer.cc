#include "ReputationListSerializer.h"

#include <string.h>
#include <iostream>
/*******************************************
 * Known bugs: 
 *    1. Currently only supporting serialization of strings of length upto 255(can be store
 *       in a byte) -> this is because when serializing a smaller integer using a writeUint32Be
 *       writes the rest of the zero bytes as \0 and then the code assumes the string gets finished
 *       at that \0.           
 * *****************************************/

void serializeReputationList(MemoryOutputStream &stream, TrustManager *trustList) {
    TrustNodeList* tmpNode = trustList->head;
    int numEntries = 0;
    while(tmpNode != nullptr) {
        numEntries++;
        tmpNode = tmpNode->next;
    }

    tmpNode = trustList->head;

    //Write number of entries in trustNodeList into the stream
    stream.writeByte(numEntries);

    while(tmpNode != nullptr) {
        //write nodeId
        stream.writeByte(tmpNode->nodeId.length());
        stream.writeBytes(reinterpret_cast<const uint8_t*>(tmpNode->nodeId.c_str()), B(tmpNode->nodeId.length()));

        //write last calculated reputation of the node
        string reputationStr = to_string(tmpNode->lastCalculatedReputation);
        stream.writeByte(reputationStr.length());
        stream.writeBytes(reinterpret_cast<const uint8_t*>(reputationStr.c_str()), B(reputationStr.length()));

        stream.writeByte(tmpNode->timestamp);
    }
}


void deserializeReputationList(MemoryInputStream &stream, TrustManager *trustList) {
    int numEntries = stream.readByte();
    std::vector<uint8_t> tmp;

    for(int i = 0; i < numEntries; i++) {
        string nodeId, reputationStr;
        int nodeIdLen, reputationStrLen;

        nodeIdLen = stream.readByte();
        tmp.clear();
        stream.readBytes(tmp, B(nodeIdLen));
        nodeId = string(tmp.begin(), tmp.end());


        reputationStrLen = stream.readByte();
        tmp.clear();
        stream.readBytes(tmp, B(reputationStrLen));
        reputationStr = string(tmp.begin(), tmp.end());

        std::string::size_type sz;
        double reputation = std::stod(reputationStr, &sz);

        uint8_t lastUpdatedTimestamp = stream.readByte();

        //Setting the direct reputation and the last calculated reputation of the car to the
        //same value
        trustList->updateTrustValue(nodeId, reputation, reputation, lastUpdatedTimestamp);
    }
}