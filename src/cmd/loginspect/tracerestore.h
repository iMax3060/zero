#ifndef __TRACERESTORE_H
#define __TRACERESTORE_H

#include "command.h"
#include "handler.h"

class RestoreTrace : public LogScannerCommand {
public:
    void run();
    void setupOptions();
};

class RestoreTraceHandler : public Handler {
public:
    RestoreTraceHandler();
    virtual void invoke(logrec_t& r);
protected:
    int currentTick;
};

#endif // __TRACERESTORE_H
