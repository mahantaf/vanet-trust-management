#ifndef __REPUTATION_SERIALIZER_H
#define __REPUTATION_SERIALIZER_H


#include "inet/common/MemoryInputStream.h"
#include "inet/common/MemoryOutputStream.h"
#include "apps/trustManager/TrustManager.h"

//Serializes the given trust list
void serializeReputationList(MemoryOutputStream &stream, TrustManager *trustList);

//Deserializes and updates the reputation values from the list into the provided trust list
void deserializeAndUpdateReputationList(MemoryInputStream &stream, TrustManager *trustList);

#endif