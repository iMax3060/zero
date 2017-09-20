#ifndef __LOGRECINFO_H
#define __LOGRECINFO_H

#include "command.h"

class LogrecInfo : public LogScannerCommand {
public:
    void usage();
    void run();
    void setupOptions();
};

#endif // __LOGRECINFO_H
