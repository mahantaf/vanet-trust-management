//=========================================================================
// EVENTLOGWRITER.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//  This is a generated file -- do not modify.
//
//=========================================================================


#include "eventlogwriter.h"
#include "common/stringutil.h"
#include "omnetpp/cconfigoption.h"
#include "omnetpp/csimulation.h"
#include "omnetpp/cmodule.h"
#include "omnetpp/cexception.h"

#ifdef CHECK
#undef CHECK
#endif
#define CHECK(fprintf)    if (fprintf<0) throw cRuntimeError("Cannot write event log file, disk full?");

namespace omnetpp {
namespace envir {

using namespace omnetpp::common;

void EventLogWriter::recordLogLine(FILE *f, const char *prefix, const char *line, int lineLength)
{
    CHECK(fprintf(f, "- %s", prefix));
    CHECK(fwrite(line, 1, lineLength, f));
}

void EventLogWriter::recordSimulationBeginEntry_v_rid_b(FILE *f, int version, const char * runId, int keyframeBlockSize)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "SB v %d rid %s b %d\n", version, QUOTE(runId), keyframeBlockSize));
}

void EventLogWriter::recordSimulationEndEntry_e_c_m(FILE *f, bool isError, int resultCode, const char * message)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "SE e %d c %d m %s\n", isError, resultCode, QUOTE(message)));
}

void EventLogWriter::recordBubbleEntry_id_txt(FILE *f, int moduleId, const char * text)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "BU id %d txt %s\n", moduleId, QUOTE(text)));
}

void EventLogWriter::recordModuleMethodBeginEntry_sm_tm_m(FILE *f, int fromModuleId, int toModuleId, const char * method)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "MB sm %d tm %d m %s\n", fromModuleId, toModuleId, QUOTE(method)));
}

void EventLogWriter::recordModuleMethodEndEntry(FILE *f)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "ME\n"));
}

void EventLogWriter::recordModuleCreatedEntry_id_c_t_n(FILE *f, int moduleId, const char * moduleClassName, const char * nedTypeName, const char * fullName)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "MC id %d c %s t %s n %s\n", moduleId, QUOTE(moduleClassName), QUOTE(nedTypeName), QUOTE(fullName)));
}

void EventLogWriter::recordModuleCreatedEntry_id_c_t_pid_n_cm(FILE *f, int moduleId, const char * moduleClassName, const char * nedTypeName, int parentModuleId, const char * fullName, bool compoundModule)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "MC id %d c %s t %s", moduleId, QUOTE(moduleClassName), QUOTE(nedTypeName)));
    if (parentModuleId!=-1)
        CHECK(fprintf(f, " pid %d", parentModuleId));
    CHECK(fprintf(f, " n %s", QUOTE(fullName)));
    if (compoundModule!=false)
        CHECK(fprintf(f, " cm %d", compoundModule));
    CHECK(fprintf(f, "\n"));
}

void EventLogWriter::recordModuleDeletedEntry_id(FILE *f, int moduleId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "MD id %d\n", moduleId));
}

void EventLogWriter::recordGateCreatedEntry_m_g_n_o(FILE *f, int moduleId, int gateId, const char * name, bool isOutput)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "GC m %d g %d n %s o %d\n", moduleId, gateId, QUOTE(name), isOutput));
}

void EventLogWriter::recordGateCreatedEntry_m_g_n_i_o(FILE *f, int moduleId, int gateId, const char * name, int index, bool isOutput)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "GC m %d g %d n %s", moduleId, gateId, QUOTE(name)));
    if (index!=-1)
        CHECK(fprintf(f, " i %d", index));
    CHECK(fprintf(f, " o %d\n", isOutput));
}

void EventLogWriter::recordGateDeletedEntry_m_g(FILE *f, int moduleId, int gateId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "GD m %d g %d\n", moduleId, gateId));
}

void EventLogWriter::recordConnectionCreatedEntry_sm_sg_dm_dg(FILE *f, int sourceModuleId, int sourceGateId, int destModuleId, int destGateId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CC sm %d sg %d dm %d dg %d\n", sourceModuleId, sourceGateId, destModuleId, destGateId));
}

void EventLogWriter::recordConnectionDeletedEntry_sm_sg(FILE *f, int sourceModuleId, int sourceGateId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CD sm %d sg %d\n", sourceModuleId, sourceGateId));
}

void EventLogWriter::recordConnectionDisplayStringChangedEntry_sm_sg_d(FILE *f, int sourceModuleId, int sourceGateId, const char * displayString)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CS sm %d sg %d d %s\n", sourceModuleId, sourceGateId, QUOTE(displayString)));
}

void EventLogWriter::recordModuleDisplayStringChangedEntry_id_d(FILE *f, int moduleId, const char * displayString)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "MS id %d d %s\n", moduleId, QUOTE(displayString)));
}

void EventLogWriter::recordEventEntry_e_t_m_ce_msg(FILE *f, eventnumber_t eventNumber, simtime_t simulationTime, int moduleId, eventnumber_t causeEventNumber, long messageId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "E # %" PRId64 " t %s m %d ce %" PRId64 " msg %ld\n", eventNumber, SIMTIME_STR(simulationTime), moduleId, causeEventNumber, messageId));
}

void EventLogWriter::recordEventEntry_e_t_m_ce_msg_f(FILE *f, eventnumber_t eventNumber, simtime_t simulationTime, int moduleId, eventnumber_t causeEventNumber, long messageId, const char * fingerprints)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "E # %" PRId64 " t %s m %d ce %" PRId64 " msg %ld", eventNumber, SIMTIME_STR(simulationTime), moduleId, causeEventNumber, messageId));
    if (fingerprints!=NULL)
        CHECK(fprintf(f, " f %s", QUOTE(fingerprints)));
    CHECK(fprintf(f, "\n"));
}

void EventLogWriter::recordKeyframeEntry_p_c_s(FILE *f, int64_t previousKeyframeFileOffset, const char * consequenceLookaheadLimits, const char * simulationStateEntries)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "KF p %" PRId64 " c %s s %s\n", previousKeyframeFileOffset, QUOTE(consequenceLookaheadLimits), QUOTE(simulationStateEntries)));
}

void EventLogWriter::recordMessageEntry_id_tid_eid_etid_c_n_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "abstract id %ld tid %ld eid %ld etid %ld c %s n %s pe %" PRId64 "\n", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName), previousEventNumber));
}

void EventLogWriter::recordMessageEntry_id_tid_eid_etid_c_n_k_p_l_er_d_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, short messageKind, short messagePriority, int64_t messageLength, bool hasBitError, const char * detail, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "abstract id %ld tid %ld eid %ld etid %ld c %s n %s", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName)));
    if (messageKind!=0)
        CHECK(fprintf(f, " k %d", messageKind));
    if (messagePriority!=0)
        CHECK(fprintf(f, " p %d", messagePriority));
    if (messageLength!=0)
        CHECK(fprintf(f, " l %" PRId64 "", messageLength));
    if (hasBitError!=false)
        CHECK(fprintf(f, " er %d", hasBitError));
    if (detail!=NULL)
        CHECK(fprintf(f, " d %s", QUOTE(detail)));
    CHECK(fprintf(f, " pe %" PRId64 "\n", previousEventNumber));
}

void EventLogWriter::recordCancelEventEntry_id_tid_eid_etid_c_n_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CE id %ld tid %ld eid %ld etid %ld c %s n %s pe %" PRId64 "\n", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName), previousEventNumber));
}

void EventLogWriter::recordCancelEventEntry_id_tid_eid_etid_c_n_k_p_l_er_d_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, short messageKind, short messagePriority, int64_t messageLength, bool hasBitError, const char * detail, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CE id %ld tid %ld eid %ld etid %ld c %s n %s", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName)));
    if (messageKind!=0)
        CHECK(fprintf(f, " k %d", messageKind));
    if (messagePriority!=0)
        CHECK(fprintf(f, " p %d", messagePriority));
    if (messageLength!=0)
        CHECK(fprintf(f, " l %" PRId64 "", messageLength));
    if (hasBitError!=false)
        CHECK(fprintf(f, " er %d", hasBitError));
    if (detail!=NULL)
        CHECK(fprintf(f, " d %s", QUOTE(detail)));
    CHECK(fprintf(f, " pe %" PRId64 "\n", previousEventNumber));
}

void EventLogWriter::recordBeginSendEntry_id_tid_eid_etid_c_n_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "BS id %ld tid %ld eid %ld etid %ld c %s n %s pe %" PRId64 "\n", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName), previousEventNumber));
}

void EventLogWriter::recordBeginSendEntry_id_tid_eid_etid_c_n_k_p_l_er_d_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, short messageKind, short messagePriority, int64_t messageLength, bool hasBitError, const char * detail, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "BS id %ld tid %ld eid %ld etid %ld c %s n %s", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName)));
    if (messageKind!=0)
        CHECK(fprintf(f, " k %d", messageKind));
    if (messagePriority!=0)
        CHECK(fprintf(f, " p %d", messagePriority));
    if (messageLength!=0)
        CHECK(fprintf(f, " l %" PRId64 "", messageLength));
    if (hasBitError!=false)
        CHECK(fprintf(f, " er %d", hasBitError));
    if (detail!=NULL)
        CHECK(fprintf(f, " d %s", QUOTE(detail)));
    CHECK(fprintf(f, " pe %" PRId64 "\n", previousEventNumber));
}

void EventLogWriter::recordEndSendEntry_t(FILE *f, simtime_t arrivalTime)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "ES t %s\n", SIMTIME_STR(arrivalTime)));
}

void EventLogWriter::recordEndSendEntry_t_is(FILE *f, simtime_t arrivalTime, bool isReceptionStart)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "ES t %s", SIMTIME_STR(arrivalTime)));
    if (isReceptionStart!=false)
        CHECK(fprintf(f, " is %d", isReceptionStart));
    CHECK(fprintf(f, "\n"));
}

void EventLogWriter::recordSendDirectEntry_sm_dm_dg(FILE *f, int senderModuleId, int destModuleId, int destGateId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "SD sm %d dm %d dg %d\n", senderModuleId, destModuleId, destGateId));
}

void EventLogWriter::recordSendDirectEntry_sm_dm_dg_pd_td(FILE *f, int senderModuleId, int destModuleId, int destGateId, simtime_t propagationDelay, simtime_t transmissionDelay)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "SD sm %d dm %d dg %d", senderModuleId, destModuleId, destGateId));
    if (propagationDelay!=0)
        CHECK(fprintf(f, " pd %s", SIMTIME_STR(propagationDelay)));
    if (transmissionDelay!=0)
        CHECK(fprintf(f, " td %s", SIMTIME_STR(transmissionDelay)));
    CHECK(fprintf(f, "\n"));
}

void EventLogWriter::recordSendHopEntry_sm_sg(FILE *f, int senderModuleId, int senderGateId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "SH sm %d sg %d\n", senderModuleId, senderGateId));
}

void EventLogWriter::recordSendHopEntry_sm_sg_pd_td_del(FILE *f, int senderModuleId, int senderGateId, simtime_t propagationDelay, simtime_t transmissionDelay, bool discard)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "SH sm %d sg %d", senderModuleId, senderGateId));
    if (propagationDelay!=0)
        CHECK(fprintf(f, " pd %s", SIMTIME_STR(propagationDelay)));
    if (transmissionDelay!=0)
        CHECK(fprintf(f, " td %s", SIMTIME_STR(transmissionDelay)));
    if (discard!=false)
        CHECK(fprintf(f, " del %d", discard));
    CHECK(fprintf(f, "\n"));
}

void EventLogWriter::recordCreateMessageEntry_id_tid_eid_etid_c_n_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CM id %ld tid %ld eid %ld etid %ld c %s n %s pe %" PRId64 "\n", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName), previousEventNumber));
}

void EventLogWriter::recordCreateMessageEntry_id_tid_eid_etid_c_n_k_p_l_er_d_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, short messageKind, short messagePriority, int64_t messageLength, bool hasBitError, const char * detail, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CM id %ld tid %ld eid %ld etid %ld c %s n %s", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName)));
    if (messageKind!=0)
        CHECK(fprintf(f, " k %d", messageKind));
    if (messagePriority!=0)
        CHECK(fprintf(f, " p %d", messagePriority));
    if (messageLength!=0)
        CHECK(fprintf(f, " l %" PRId64 "", messageLength));
    if (hasBitError!=false)
        CHECK(fprintf(f, " er %d", hasBitError));
    if (detail!=NULL)
        CHECK(fprintf(f, " d %s", QUOTE(detail)));
    CHECK(fprintf(f, " pe %" PRId64 "\n", previousEventNumber));
}

void EventLogWriter::recordCloneMessageEntry_id_tid_eid_etid_c_n_pe_cid(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, eventnumber_t previousEventNumber, long cloneId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CL id %ld tid %ld eid %ld etid %ld c %s n %s pe %" PRId64 " cid %ld\n", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName), previousEventNumber, cloneId));
}

void EventLogWriter::recordCloneMessageEntry_id_tid_eid_etid_c_n_k_p_l_er_d_pe_cid(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, short messageKind, short messagePriority, int64_t messageLength, bool hasBitError, const char * detail, eventnumber_t previousEventNumber, long cloneId)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "CL id %ld tid %ld eid %ld etid %ld c %s n %s", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName)));
    if (messageKind!=0)
        CHECK(fprintf(f, " k %d", messageKind));
    if (messagePriority!=0)
        CHECK(fprintf(f, " p %d", messagePriority));
    if (messageLength!=0)
        CHECK(fprintf(f, " l %" PRId64 "", messageLength));
    if (hasBitError!=false)
        CHECK(fprintf(f, " er %d", hasBitError));
    if (detail!=NULL)
        CHECK(fprintf(f, " d %s", QUOTE(detail)));
    CHECK(fprintf(f, " pe %" PRId64 " cid %ld\n", previousEventNumber, cloneId));
}

void EventLogWriter::recordDeleteMessageEntry_id_tid_eid_etid_c_n_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "DM id %ld tid %ld eid %ld etid %ld c %s n %s pe %" PRId64 "\n", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName), previousEventNumber));
}

void EventLogWriter::recordDeleteMessageEntry_id_tid_eid_etid_c_n_k_p_l_er_d_pe(FILE *f, long messageId, long messageTreeId, long messageEncapsulationId, long messageEncapsulationTreeId, const char * messageClassName, const char * messageName, short messageKind, short messagePriority, int64_t messageLength, bool hasBitError, const char * detail, eventnumber_t previousEventNumber)
{
    ASSERT(f!=nullptr);
    CHECK(fprintf(f, "DM id %ld tid %ld eid %ld etid %ld c %s n %s", messageId, messageTreeId, messageEncapsulationId, messageEncapsulationTreeId, QUOTE(messageClassName), QUOTE(messageName)));
    if (messageKind!=0)
        CHECK(fprintf(f, " k %d", messageKind));
    if (messagePriority!=0)
        CHECK(fprintf(f, " p %d", messagePriority));
    if (messageLength!=0)
        CHECK(fprintf(f, " l %" PRId64 "", messageLength));
    if (hasBitError!=false)
        CHECK(fprintf(f, " er %d", hasBitError));
    if (detail!=NULL)
        CHECK(fprintf(f, " d %s", QUOTE(detail)));
    CHECK(fprintf(f, " pe %" PRId64 "\n", previousEventNumber));
}


} // namespace envir

}  // namespace omnetpp
