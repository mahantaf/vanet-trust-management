//=========================================================================
//  EVENTLOGENTRY.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//  Author: Levente Meszaros
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include "eventlog.h"
#include "eventlogentry.h"
#include "eventlogentryfactory.h"

namespace omnetpp {
namespace eventlog {

char EventLogEntry::buffer[128];
omnetpp::common::LineTokenizer EventLogEntry::tokenizer(32768);
static const char *currentLine;
static int currentLineLength;

/***********************************************/

EventLogEntry::EventLogEntry()
{
    contextModuleId = -1;
    level = -1;
    event = nullptr;
    entryIndex = -1;
}

EventLogEntry *EventLogEntry::parseEntry(EventLog *eventLog, Event *event, int entryIndex, file_offset_t offset, char *line, int length)
{
    try {
        currentLine = line;
        currentLineLength = length;

        if (*line == '-') {
            EventLogMessageEntry *eventLogMessage = new EventLogMessageEntry(event, entryIndex);
            eventLogMessage->parse(line, length);
            return eventLogMessage;
        }
        else {
            EventLogEntryFactory factory;
            tokenizer.tokenize(line, length);
            Assert(entryIndex >= 0);
            return factory.parseEntry(event, entryIndex, tokenizer.tokens(), tokenizer.numTokens());
        }
    }
    catch (opp_runtime_error& e) {
        const char *fileName = eventLog->getFileReader()->getFileName();
        if (event && event->getEventEntry())
            throw opp_runtime_error("Error parsing elog file %s at line %d of event #%" EVENTNUMBER_PRINTF_FORMAT " near file offset %" PRId64 ":\n%s", fileName, entryIndex, event->getEventNumber(), offset, e.what());
        else
            throw opp_runtime_error("Error parsing elog file %s near file offset %" PRId64 ":\n%s", fileName, offset, e.what());
    }
}

eventnumber_t EventLogEntry::parseEventNumber(const char *str)
{
    char *end;
    return strtoll(str, &end, 10);
}

simtime_t EventLogEntry::parseSimulationTime(const char *str)
{
    const char *end;
    return BigDecimal::parse(str, end);
}

/***********************************************/

char *EventLogTokenBasedEntry::getToken(char **tokens, int numTokens, const char *sign, bool mandatory)
{
    for (int i = 1; i < numTokens; i += 2)
        if (!strcmp(tokens[i], sign))
            return tokens[i + 1];

    if (mandatory)
        throw opp_runtime_error("Missing mandatory token %s in line: %.*s", sign, currentLineLength, currentLine);

    return nullptr;
}

bool EventLogTokenBasedEntry::getBoolToken(char **tokens, int numTokens, const char *sign, bool mandatory, bool defaultValue)
{
    int value = getIntToken(tokens, numTokens, sign, mandatory, defaultValue);

    if (value == 0)
        return false;
    else if (value == 1)
        return true;
    else
        throw opp_runtime_error("Invalid boolean %d in line %.*s", value, currentLineLength, currentLine);
}

int EventLogTokenBasedEntry::getIntToken(char **tokens, int numTokens, const char *sign, bool mandatory, int defaultValue)
{
    char *token = getToken(tokens, numTokens, sign, mandatory);

    if (token) {
        errno = 0;
        int value = atoi(token);
        if (errno)
            throw opp_runtime_error("Invalid int %s in line: %.*s", token, currentLineLength, currentLine);
        return value;
    }
    else
        return defaultValue;
}

short EventLogTokenBasedEntry::getShortToken(char **tokens, int numTokens, const char *sign, bool mandatory, short defaultValue)
{
    char *token = getToken(tokens, numTokens, sign, mandatory);

    if (token) {
        errno = 0;
        short value = atoi(token);
        if (errno)
            throw opp_runtime_error("Invalid short %s in line: %.*s", token, currentLineLength, currentLine);
        return value;
    }
    else
        return defaultValue;
}

long EventLogTokenBasedEntry::getLongToken(char **tokens, int numTokens, const char *sign, bool mandatory, long defaultValue)
{
    char *token = getToken(tokens, numTokens, sign, mandatory);

    if (token) {
        errno = 0;
        long value = atol(token);
        if (errno)
            throw opp_runtime_error("Invalid long %s in line: %.*s", token, currentLineLength, currentLine);
        return value;
    }
    else
        return defaultValue;
}

int64_t EventLogTokenBasedEntry::getInt64Token(char **tokens, int numTokens, const char *sign, bool mandatory, int64_t defaultValue)
{
    char *token = getToken(tokens, numTokens, sign, mandatory);

    if (token) {
        char *end;
        errno = 0;
        int64_t value = strtoll(token, &end, 10);
        if (errno)
            throw opp_runtime_error("Invalid int64_t %s in line: %.*s", token, currentLineLength, currentLine);
        return value;
    }
    else
        return defaultValue;
}

eventnumber_t EventLogTokenBasedEntry::getEventNumberToken(char **tokens, int numTokens, const char *sign, bool mandatory, eventnumber_t defaultValue)
{
    char *token = getToken(tokens, numTokens, sign, mandatory);

    if (token) {
        char *end;
        errno = 0;
        int64_t value = strtoll(token, &end, 10);
        if (errno)
            throw opp_runtime_error("Invalid event number %s in line: %.*s", token, currentLineLength, currentLine);
        return value;
    }
    else
        return defaultValue;
}

simtime_t EventLogTokenBasedEntry::getSimtimeToken(char **tokens, int numTokens, const char *sign, bool mandatory, simtime_t defaultValue)
{
    char *token = getToken(tokens, numTokens, sign, mandatory);

    if (token) {
        const char *end;
        BigDecimal value = BigDecimal::parse(token, end);
        if (*end != '\0')
            throw opp_runtime_error("Invalid simulation time %s in line: %.*s", token, currentLineLength, currentLine);
        return value;
    }
    else
        return defaultValue;
}

const char *EventLogTokenBasedEntry::getStringToken(char **tokens, int numTokens, const char *sign, bool mandatory, const char *defaultValue)
{
    char *token = getToken(tokens, numTokens, sign, mandatory);
    return token ? eventLogStringPool.get(token) : defaultValue;
}

void EventLogTokenBasedEntry::parse(char *line, int length)
{
    tokenizer.tokenize(line, length);
    parse(tokenizer.tokens(), tokenizer.numTokens());
}

/***********************************************/

EventLogMessageEntry::EventLogMessageEntry(Event *event, int entryIndex)
{
    this->event = event;
    this->entryIndex = entryIndex;
    text = nullptr;
}

void EventLogMessageEntry::parse(char *line, int length)
{
    // line starts with '-' and SP, length includes CR/LF
    char *s = line + length - 1;
    char ch1 = '\0';
    char ch2 = '\0';
    if (length > 0) {
        ch1 = *s;
        *s = '\0';
    }
    if (length > 1 && *(s - 1) == '\r') {
        ch2 = *(s - 1);
        *(s - 1) = '\0';
    }
    text = eventLogStringPool.get(line + 2);
    if (length > 0 && *s == '\0')
        *s = ch1;
    if (length > 1 && *(s - 1) == '\0')
        *(s - 1) = ch2;
}

void EventLogMessageEntry::print(FILE *fout)
{
    ::fprintf(fout, "- %s\n", text);
}

const std::vector<const char *> EventLogMessageEntry::getAttributeNames() const
{
    std::vector<const char *> names;
    names.push_back("-");

    return names;
}

const char *EventLogMessageEntry::getAsString(const char *attribute) const
{
    if (!strcmp(attribute, "type"))
        return "-";
    else if (!strcmp(attribute, "-"))
        return text;
    else
        return nullptr;
}

} // namespace eventlog
}  // namespace omnetpp
