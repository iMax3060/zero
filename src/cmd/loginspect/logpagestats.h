#ifndef __LOGPAGESTATS_H
#define __LOGPAGESTATS_H

#include "command.h"

class LogPageStats : public LogScannerCommand {
public:
    void usage();

    void run();

    void setupOptions();
};

#endif // __LOGPAGESTATS_H
