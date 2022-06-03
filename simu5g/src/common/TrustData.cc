#include "TrustData.h"


/*******************************************
 * Known bugs: 
 *    1. Currently only supporting serialization of strings of length upto 255(can be store
 *       in a byte) -> this is because when serializing a smaller integer using a writeUint32Be
 *       writes the rest of the zero bytes as \0 and then the code assumes the string gets finished
 *       at that \0.           
 * *****************************************/

void TrustData::serializeTrustData(MemoryOutputStream &stream)
{
    //write generation time as a byte string, writing length of
    //string before so that while deserializing we know how many bytes to read
    const std::string &curr_time_str = this->generation_time.str();
    // std::cout << "Generation time: " << curr_time_str << ", and length of str is" << curr_time_str.length() << std::endl;
    stream.writeByte(curr_time_str.length());
    stream.writeBytes(reinterpret_cast<const uint8_t*>(curr_time_str.c_str()), B(curr_time_str.length()));

    //Write generation location as a byte string, writing length of
    //string before so that while deserializing we know how many bytes to read
    const std::string &generator_location_str = this->generator_location.str();
    stream.writeByte(generator_location_str.length());
    stream.writeBytes(reinterpret_cast<const uint8_t*>(generator_location_str.c_str()), B(generator_location_str.length()));

    //Write event location as a byte string, writing length of
    //string before so that while deserializing we know how many bytes to read
    const std::string &event_location_str = this->event_location.str();
    stream.writeByte(event_location_str.length());
    stream.writeBytes(reinterpret_cast<const uint8_t*>(event_location_str.c_str()), B(event_location_str.length()));

    //write sender id as byte string. The sender ID is the name of the car i.e. "car[0]", writing length of
    //string before so that while deserializing we know how many bytes to read
    stream.writeByte(this->senderID.length());
    stream.writeBytes(reinterpret_cast<const uint8_t*>(this->senderID.c_str()), B(this->senderID.length()));

    //write node_velocity as a byte string, writing length of
    //string before so that while deserializing we know how many bytes to read
    const std::string &node_velocity_str = this->node_velocity.str();
    stream.writeByte(node_velocity_str.length());
    stream.writeBytes(reinterpret_cast<const uint8_t*>(node_velocity_str.c_str()), B(node_velocity_str.length()));


    std::vector<uint8_t> serialized_data;
    stream.copyData(serialized_data);
    // std::cout << "Serialized message with sender: " << this->senderID << " and velocity: " << node_velocity_str << std::endl;
}

void TrustData::deserializeTrustData(MemoryInputStream& stream) 
{
    std::vector<uint8_t> tmp;

    //read generation time as a byte string
    int sizeOfGenerationTimeStr = 0;
    sizeOfGenerationTimeStr = stream.readByte();
    stream.readBytes(tmp, B(sizeOfGenerationTimeStr));
    std::string generationTimeStr(tmp.begin(), tmp.end());
    this->generation_time = simTime();

    //Read generation location as a byte string
    int sizeOfGenerationLocationStr = 0;
    sizeOfGenerationLocationStr = stream.readByte();
    tmp.clear();
    stream.readBytes(tmp, B(sizeOfGenerationLocationStr));
    std::string generationLocationStr(tmp.begin(), tmp.end());
    this->generator_location = Coord::convertStrToCoord(generationLocationStr.c_str());

    //Read event location as a byte string
    int sizeOfEventLocationStr = 0;
    sizeOfEventLocationStr = stream.readByte();
    tmp.clear();
    stream.readBytes(tmp, B(sizeOfEventLocationStr));
    std::string eventLocationStr(tmp.begin(), tmp.end());
    this->event_location = Coord::convertStrToCoord(eventLocationStr.c_str());

    //read sender id as byte string. The sender ID is the name of the car i.e. "car[0]"
    int sizeOfSenderIDStr = 0;
    sizeOfSenderIDStr = stream.readByte();
    tmp.clear();
    stream.readBytes(tmp, B(sizeOfSenderIDStr));
    std::string senderIDStr(tmp.begin(), tmp.end());
    this->senderID = senderIDStr;

    //read node velocity as byte string into inet::Coord
    int sizeOfNodeVelocityStr = 0;
    sizeOfNodeVelocityStr = stream.readByte();
    tmp.clear();
    stream.readBytes(tmp, B(sizeOfNodeVelocityStr));
    std::string nodeVelocityStr(tmp.begin(), tmp.end());
    this->node_velocity = Coord::convertStrToCoord(nodeVelocityStr.c_str());
}

//Two trust message contents are same if the event they are describing is the same
bool operator==(TrustData &tm1, TrustData &tm2) {
    return tm1.getEventLocation() == tm2.getEventLocation();
}
