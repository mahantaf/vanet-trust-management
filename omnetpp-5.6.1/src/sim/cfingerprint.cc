//==========================================================================
//   CFINGERPRINT.CC  - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2008 Andras Varga
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "omnetpp/cfingerprint.h"
#include "omnetpp/csimulation.h"
#include "omnetpp/cmodule.h"
#include "omnetpp/cpacket.h"
#include "omnetpp/ccomponenttype.h"
#include "omnetpp/cclassdescriptor.h"
#include "omnetpp/crng.h"
#include "omnetpp/cstatistic.h"
#include "omnetpp/cabstracthistogram.h"
#include "omnetpp/cstringtokenizer.h"
#include "omnetpp/cconfiguration.h"
#include "omnetpp/cconfigoption.h"
#include "omnetpp/regmacros.h"
#include "common/stringutil.h"

#ifdef WITH_PARSIM
#include "parsim/cmemcommbuffer.h"
#endif

namespace omnetpp {

#ifdef USE_OMNETPP4x_FINGERPRINTS

Register_Class(cOmnetpp4xFingerprintCalculator);

cOmnetpp4xFingerprintCalculator::cOmnetpp4xFingerprintCalculator()
{
    hasher = nullptr;
}

cOmnetpp4xFingerprintCalculator::~cOmnetpp4xFingerprintCalculator()
{
    delete hasher;
}

void cOmnetpp4xFingerprintCalculator::initialize(const char *expectedFingerprints, cConfiguration *cfg, int index)
{
    this->expectedFingerprints = expectedFingerprints;
    hasher = new cHasher();
}

void cOmnetpp4xFingerprintCalculator::addEvent(cEvent *event)
{
    if (event->isMessage()) {
        cMessage *message = static_cast<cMessage *>(event);
        cModule *module = message->getArrivalModule();
        hasher->add(simTime().raw());
        hasher->add(module->getVersion4ModuleId());
    }
}

bool cOmnetpp4xFingerprintCalculator::checkFingerprint() const
{
    cStringTokenizer tokenizer(expectedFingerprints.c_str());
    while (tokenizer.hasMoreTokens()) {
        std::string fingerprint = tokenizer.nextToken();
        if (fingerprint.find('/') != std::string::npos)
            continue;   // skip omnetpp 5.0 fingerprints
        if (hasher->equals(fingerprint.c_str()))
            return true;
    }
    return false;
}

#else // if !USE_OMNETPP4x_FINGERPRINTS

Register_Class(cSingleFingerprintCalculator);

Register_PerRunConfigOption(CFGID_FINGERPRINT_INGREDIENTS, "fingerprint-ingredients", CFG_STRING, "tplx", "Specifies the list of ingredients to be taken into account for fingerprint computation. Each character corresponds to one ingredient: 'e' event number, 't' simulation time, 'n' message (event) full name, 'c' message (event) class name, 'k' message kind, 'l' message bit length, 'o' message control info class name, 'd' message data, 'i' module id, 'm' module full name, 'p' module full path, 'a' module class name, 'r' random numbers drawn, 's' scalar results, 'z' statistic results, 'v' vector results, 'x' extra data provided by modules. Note: ingredients specified in an expected fingerprint (characters after the '/' in the fingerprint value) take precedence over this setting. If you configured multiple fingerprints, separate ingredients with commas.");
Register_PerRunConfigOption(CFGID_FINGERPRINT_EVENTS, "fingerprint-events", CFG_STRING, "*", "Configures the fingerprint calculator to consider only certain events. The value is a pattern that will be matched against the event name by default. It may also be an expression containing pattern matching characters, field access, and logical operators. The default setting is '*' which includes all events in the calculated fingerprint. If you configured multiple fingerprints, separate filters with commas.");
Register_PerRunConfigOption(CFGID_FINGERPRINT_MODULES, "fingerprint-modules", CFG_STRING, "*", "Configures the fingerprint calculator to consider only certain modules. The value is a pattern that will be matched against the module full path by default. It may also be an expression containing pattern matching characters, field access, and logical operators. The default setting is '*' which includes all events in all modules in the calculated fingerprint. If you configured multiple fingerprints, separate filters with commas.");
Register_PerRunConfigOption(CFGID_FINGERPRINT_RESULTS, "fingerprint-results", CFG_STRING, "*", "Configures the fingerprint calculator to consider only certain results. The value is a pattern that will be matched against the result full path by default. It may also be an expression containing pattern matching characters, field access, and logical operators. The default setting is '*' which includes all results in all modules in the calculated fingerprint. If you configured multiple fingerprints, separate filters with commas.");

const char *cSingleFingerprintCalculator::MatchableObject::getAsString() const
{
    attributeValue = object->getFullPath();
    return attributeValue.c_str();
}

const char *cSingleFingerprintCalculator::MatchableObject::getAsString(const char *attribute) const
{
    cClassDescriptor *descriptor = const_cast<cObject *>(object)->getDescriptor();
    int fieldId = descriptor->findField(attribute);
    if (fieldId == -1)
        return nullptr;
    else {
        attributeValue = descriptor->getFieldValueAsString(const_cast<cObject *>(object), fieldId, 0);
        return attributeValue.c_str();
    }
}

cSingleFingerprintCalculator::cSingleFingerprintCalculator()
{
    eventMatcher = nullptr;
    moduleMatcher = nullptr;
    resultMatcher = nullptr;
    hasher = nullptr;
    addEvents = false;
    addScalarResults = false;
    addStatisticResults = false;
    addVectorResults = false;
    addExtraData_ = false;
}

cSingleFingerprintCalculator::~cSingleFingerprintCalculator()
{
    delete eventMatcher;
    delete moduleMatcher;
    delete resultMatcher;
    delete hasher;
}

inline std::string getListItem(const std::string& list, int index)
{
    std::vector<std::string> items = cStringTokenizer(list.c_str(), ",").asVector();
    return (index >= 0 && index < (int)items.size()) ? items[index] : "";
}

void cSingleFingerprintCalculator::initialize(const char *expectedFingerprints, cConfiguration *cfg, int index)
{
    this->expectedFingerprints = expectedFingerprints;
    hasher = new cHasher();

    // fingerprints may have an ingredients string embedded in them after a "/" character;
    // if so, that overrides the fingerprint-ingredients configuration option.
    std::string options;
    cStringTokenizer tokenizer(expectedFingerprints);
    while (tokenizer.hasMoreTokens()) {
        const char *fingerprint = tokenizer.nextToken();
        const char *slash = strchr(fingerprint, '/');
        if (slash) {
            std::string currentOptions = slash+1;
            if (options.empty())
                options = currentOptions;
            else if (options != currentOptions)
                throw cRuntimeError("Fingerprint option suffixes (parts after the '/') must agree"); //TODO better msg
        }
    }

    // parse configuration
    if (index == -1)
        index = 0;
    parseIngredients(!options.empty() ? options.c_str() : getListItem(cfg->getAsString(CFGID_FINGERPRINT_INGREDIENTS), index).c_str());
    parseEventMatcher(getListItem(cfg->getAsString(CFGID_FINGERPRINT_EVENTS), index).c_str());
    parseModuleMatcher(getListItem(cfg->getAsString(CFGID_FINGERPRINT_MODULES), index).c_str());
    parseResultMatcher(getListItem(cfg->getAsString(CFGID_FINGERPRINT_RESULTS), index).c_str());
}

std::string cSingleFingerprintCalculator::str() const
{
    return hasher->str() + "/" + ingredients;
}

cSingleFingerprintCalculator::FingerprintIngredient cSingleFingerprintCalculator::validateIngredient(char ch)
{
    if (strchr("etncklodimparszvx0", ch) == nullptr)
        throw cRuntimeError("Unknown fingerprint ingredient character '%c'", ch);
    return (FingerprintIngredient) ch;
}

void cSingleFingerprintCalculator::parseIngredients(const char *s)
{
    ingredients = s;
    for (; *s; s++) {
        char ch = *s;
        switch (validateIngredient(ch)) {
            case RESULT_SCALAR: addScalarResults = true; break;
            case RESULT_STATISTIC: addStatisticResults = true; break;
            case RESULT_VECTOR: addVectorResults = true; break;
            case EXTRA_DATA: addExtraData_ = true; break;
            default: addEvents = true;
        }
    }
}

void cSingleFingerprintCalculator::parseEventMatcher(const char *s)
{
    if (s && *s && strcmp("*", s) != 0) {
        eventMatcher = new cMatchExpression();
        eventMatcher->setPattern(s, true, true, true);
    }
}

void cSingleFingerprintCalculator::parseModuleMatcher(const char *s)
{
    if (s && *s && strcmp("*", s)) {
        moduleMatcher = new cMatchExpression();
        moduleMatcher->setPattern(s, true, true, true);
    }
}

void cSingleFingerprintCalculator::parseResultMatcher(const char *s)
{
    if (s && *s && strcmp("*", s)) {
        resultMatcher = new cMatchExpression();
        resultMatcher->setPattern(s, true, true, true);
    }
}

void cSingleFingerprintCalculator::addEvent(cEvent *event)
{
    if (addEvents) {
        const MatchableObject matchableEvent(event);
        if (eventMatcher == nullptr || eventMatcher->matches(&matchableEvent)) {
            cMessage *message = nullptr;
            cPacket *packet = nullptr;
            cObject *controlInfo = nullptr;
            cModule *module = nullptr;
            if (event->isMessage()) {
                message = static_cast<cMessage *>(event);
                if (message->isPacket())
                    packet = static_cast<cPacket *>(message);
                controlInfo = message->getControlInfo();
                module = message->getArrivalModule();
            }

            MatchableObject matchableModule(module);
            if (module == nullptr || moduleMatcher == nullptr || moduleMatcher->matches(&matchableModule)) {
                for (char & ch : ingredients) {
                    FingerprintIngredient ingredient = (FingerprintIngredient) ch;
                    if (!addEventIngredient(event, ingredient)) {
                        switch (ingredient) {
                            case EVENT_NUMBER:
                                hasher->add(getSimulation()->getEventNumber()); break;
                            case SIMULATION_TIME:
                                hasher->add(simTime().raw()); break;
                            case MESSAGE_FULL_NAME:
                                hasher->add(event->getFullName()); break;
                            case MESSAGE_CLASS_NAME:
                                hasher->add(event->getClassName()); break;
                            case MESSAGE_KIND:
                                if (message != nullptr)
                                    hasher->add(message->getKind());
                                break;
                            case MESSAGE_BIT_LENGTH:
                                if (packet != nullptr)
                                    hasher->add(packet->getBitLength());
                                break;
                            case MESSAGE_CONTROL_INFO_CLASS_NAME:
                                if (controlInfo != nullptr)
                                    hasher->add(controlInfo->getClassName());
                                break;
                            case MESSAGE_DATA:
                                if (message != nullptr) {
                                    // NOTE: workaround for control info and context pointer which cannot be packed
                                    // TODO: we should rather use a network byte order serialization API
#ifdef WITH_PARSIM
                                    cMemCommBuffer buffer;
                                    cMessage *copy = message->dup();
                                    copy->parsimPack(&buffer);
                                    hasher->add(buffer.getBuffer(), buffer.getMessageSize());
                                    delete copy;
#else
                                    throw cRuntimeError("Fingerprint is configured to contain MESSAGE_DATA (d),"
                                                        " but parallel simulation support is disabled (WITH_PARSIM=no)"
                                                        " which is required for serialization.");
#endif
                                }
                                break;
                            case MODULE_ID:
                                if (module != nullptr)
                                    hasher->add(module->getId());
                                break;
                            case MODULE_FULL_NAME:
                                if (module != nullptr)
                                    hasher->add(module->getFullName());
                                break;
                            case MODULE_FULL_PATH:
                                if (module != nullptr)
                                    hasher->add(module->getFullPath().c_str());
                                break;
                            case MODULE_CLASS_NAME:
                                if (module != nullptr)
                                    hasher->add(module->getComponentType()->getClassName());
                                break;
                            case RANDOM_NUMBERS_DRAWN:
                                for (int i = 0; i < getEnvir()->getNumRNGs(); i++)
                                    hasher->add(getEnvir()->getRNG(i)->getNumbersDrawn());
                                break;
                            case CLEAN_HASHER:
                                hasher->reset();
                                break;
                            case RESULT_SCALAR:
                            case RESULT_STATISTIC:
                            case RESULT_VECTOR:
                            case EXTRA_DATA:
                                // not processed here
                                break;
                            default:
                                throw cRuntimeError("Unknown fingerprint ingredient '%c' (%d)", ingredient, ingredient);
                        }
                    }
                }
            }
        }
    }
}

bool cSingleFingerprintCalculator::addEventIngredient(cEvent *event, FingerprintIngredient ingredient)
{
    return false;
}

void cSingleFingerprintCalculator::addScalarResult(const cComponent *component, const char *name, double value)
{
    if (addScalarResults) {
        MatchableObject matchableComponent(component);
        if (moduleMatcher == nullptr || moduleMatcher->matches(&matchableComponent)) {
            cNamedObject object(name);
            MatchableObject matchableResult(&object);
            if (resultMatcher == nullptr || resultMatcher->matches(&matchableResult))
                hasher->add(value);
        }
    }
}

void cSingleFingerprintCalculator::addStatisticResult(const cComponent *component, const char *name, const cStatistic *statistic)
{
    if (addStatisticResults) {
        MatchableObject matchableComponent(component);
        if (moduleMatcher == nullptr || moduleMatcher->matches(&matchableComponent)) {
            MatchableObject matchableResult(statistic);
            if (resultMatcher == nullptr || resultMatcher->matches(&matchableResult)) {
                hasher->add(statistic->getSumWeights());
                hasher->add(statistic->getWeightedSum());
                hasher->add(statistic->getMin());
                hasher->add(statistic->getMax());
                hasher->add(statistic->getMean());
                hasher->add(statistic->getStddev());
                if (const cAbstractHistogram *histogram = dynamic_cast<const cAbstractHistogram*>(statistic)) {
                    hasher->add(histogram->getUnderflowSumWeights());
                    hasher->add(histogram->getOverflowSumWeights());
                    int numBins = histogram->getNumBins();
                    for (int i = 0; i < numBins; i++) {
                        hasher->add(histogram->getBinEdge(i));
                        hasher->add(histogram->getBinValue(i));
                    }
                    hasher->add(histogram->getBinEdge(numBins));
                }
            }
        }
    }
}

void cSingleFingerprintCalculator::addVectorResult(const cComponent *component, const char *name, const simtime_t& t, double value)
{
    if (addVectorResults) {
        MatchableObject matchableComponent(component);
        // TODO: remove workaround for unknown component
        if (moduleMatcher == nullptr || component == nullptr || moduleMatcher->matches(&matchableComponent)) {
            cNamedObject object(name);
            MatchableObject matchableResult(&object);
            if (resultMatcher == nullptr || resultMatcher->matches(&matchableResult)) {
                hasher->add(t.raw());
                hasher->add(value);
            }
        }
    }
}

bool cSingleFingerprintCalculator::checkFingerprint() const
{
    cStringTokenizer tokenizer(expectedFingerprints.c_str());
    while (tokenizer.hasMoreTokens()) {
        std::string fingerprint = tokenizer.nextToken();
        if (fingerprint.find('/') != std::string::npos)
            fingerprint = omnetpp::common::opp_substringbefore(fingerprint, "/");
        if (hasher->equals(fingerprint.c_str()))
            return true;
    }
    return false;
}

//----

// Note: This is basically equivalent to "for (auto & element : elements)", but we don't want to rely on C++11 yet...
#define for_each_element(CODE) for (std::vector<cFingerprintCalculator *>::iterator it = elements.begin(); it != elements.end(); ++it) { cFingerprintCalculator *element = *it; CODE; }
#define for_each_element_const(CODE) for (std::vector<cFingerprintCalculator *>::const_iterator it = elements.begin(); it != elements.end(); ++it) { cFingerprintCalculator *element = *it; CODE; }

cMultiFingerprintCalculator::cMultiFingerprintCalculator(cFingerprintCalculator *prototype) :
    prototype(prototype)
{
}

cMultiFingerprintCalculator::~cMultiFingerprintCalculator()
{
    delete prototype;
    for_each_element(
        delete element;
    )
}

void cMultiFingerprintCalculator::initialize(const char *expectedFingerprintsList, cConfiguration *cfg, int index)
{
    if (index != -1)
        throw cRuntimeError("cMultiFingerprintCalculator objects cannot be nested");

    std::vector<std::string> expectedFingerprints = cStringTokenizer(expectedFingerprintsList, ",").asVector();
    for (int i = 0; i < (int)expectedFingerprints.size(); i++) {
        cFingerprintCalculator *fingerprint = static_cast<cFingerprintCalculator*>(prototype->dup());
        fingerprint->initialize(expectedFingerprints[i].c_str(), cfg, i);
        elements.push_back(fingerprint);
    }
}

void cMultiFingerprintCalculator::addEvent(cEvent *event)
{
    for_each_element(
        element->addEvent(event);
    )
}

void cMultiFingerprintCalculator::addScalarResult(const cComponent *component, const char *name, double value)
{
    for_each_element(
        element->addScalarResult(component, name, value);
    )
}

void cMultiFingerprintCalculator::addStatisticResult(const cComponent *component, const char *name, const cStatistic *value)
{
    for_each_element(
        element->addStatisticResult(component, name, value);
    )
}

void cMultiFingerprintCalculator::addVectorResult(const cComponent *component, const char *name, const simtime_t& t, double value)
{
    for_each_element(
        element->addVectorResult(component, name, t, value);
    )
}

bool cMultiFingerprintCalculator::checkFingerprint() const
{
    for_each_element_const(
        if (!element->checkFingerprint())
            return false;
    )
    return true;
}

std::string cMultiFingerprintCalculator::str() const
{
    std::stringstream stream;
    for_each_element_const(
        stream << ", " << element->str();
    );
    return stream.str().substr(2);
}

#undef for_each_element

#endif // !USE_OMNETPP4x_FINGERPRINTS

} // namespace omnetpp
