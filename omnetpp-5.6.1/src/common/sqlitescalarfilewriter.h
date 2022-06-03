//==========================================================================
//  SQLITESCALARFILEWRITER.H - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Author: Zoltan Bojthe, Andras Varga
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __OMNETPP_COMMON_SQLITESCALARFILEWRITER_H
#define __OMNETPP_COMMON_SQLITESCALARFILEWRITER_H

#include <string>
#include <map>
#include <vector>
#include "sqlite3.h"
#include "statistics.h"
#include "histogram.h"

namespace omnetpp {
namespace common {

/**
 * Class for writing SQLite-based output scalar files.
 */
class COMMON_API SqliteScalarFileWriter
{
  public:
    typedef std::map<std::string, std::string> StringMap;
    typedef std::vector<std::pair<std::string, std::string>> OrderedKeyValueList;

  protected:
    std::string fname;   // output file name
    sqlite_int64 runId;  // runId in sqlite database
    sqlite3 *db;         // sqlite database, nullptr before initialization and after error
    sqlite3_stmt *stmt;
    sqlite3_stmt *add_scalar_stmt;
    sqlite3_stmt *add_scalar_attr_stmt;
    sqlite3_stmt *add_statistic_stmt;
    sqlite3_stmt *add_statistic_attr_stmt;
    sqlite3_stmt *add_statistic_bin_stmt;

    int commitFreq=100000; // we COMMIT after every commitFreq INSERT statements
    int insertCount;

  protected:
    void prepareStatements();
    void cleanup();  // MUST NOT THROW
    void commitAndBeginNew();
    sqlite_int64 writeScalar(const std::string& componentFullPath, const std::string& name, double value);
    void writeScalarAttr(sqlite_int64 scalarId, const char *name, size_t nameLength, const char *value, size_t valueLength);
    sqlite_int64 writeStatistic(const std::string& componentFullPath, const std::string& name, const Statistics& statistic, bool isHistogram);
    void writeStatisticAttr(sqlite_int64 statisticId, const char *name, const char *value);
    void writeStatisticBin(sqlite_int64 statisticId, double lowerEdge, double binValue);
    void prepareStatement(sqlite3_stmt *&stmt, const char *sql);
    void finalizeStatement(sqlite3_stmt *&stmt);
    void checkOK(int sqlite3_result);
    void checkDone(int sqlite3_result);
    void error(const char *errmsg);

  public:
    SqliteScalarFileWriter();
    virtual ~SqliteScalarFileWriter();

    void setCommitFreq(int f) {commitFreq = f;}
    int getCommitFreq() const {return commitFreq;}

    void open(const char *filename); // append if file exists
    void close();
    bool isOpen() const {return db != nullptr;} // IMPORTANT: db will be closed when an error occurs

    void beginRecordingForRun(const std::string& runName, int simtimeScaleExp, const StringMap& attributes, const StringMap& itervars, const OrderedKeyValueList& paramAssignments);
    void endRecordingForRun();
    void recordScalar(const std::string& componentFullPath, const std::string& name, double value, const StringMap& attributes);
    void recordStatistic(const std::string& componentFullPath, const std::string& name, const Statistics& statistic, const StringMap& attributes);
    void recordHistogram(const std::string& componentFullPath, const std::string& name, const Statistics& statistic, const Histogram& bins, const StringMap& attributes);
    void flush();
};

} // namespace common
}  // namespace omnetpp

#endif
