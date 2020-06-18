#ifndef __LOGANALYSIS_H
#define __LOGANALYSIS_H

#include "command.h"
#include "handler.h"

#include <unordered_set>

class LogAnalysis : public LogScannerCommand {
public:
    void setupOptions();

    void run();

private:
    bool fullScan;

    bool takeChkpt;

    bool printPages;
};

class LogAnalysisHandler : public Handler {
public:
    LogAnalysisHandler();

    virtual ~LogAnalysisHandler() {};

    virtual void invoke(logrec_t& r);

    virtual void finalize();

    unordered_set<tid_t> activeTAs;

    unordered_set<PageID> dirtyPages;

    size_t xctCount;
};

#endif // __LOGANALYSIS_H
