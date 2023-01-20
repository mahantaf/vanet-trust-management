//
// Packet used by the ~VoIPReceiver module to signal whose remote attestation 
// is going on
//

cplusplus{{
    #include <string>
}}

message RemoteAttestationMsg {
    string sender;
}
