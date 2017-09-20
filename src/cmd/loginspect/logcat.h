#ifndef __LOGCAT_H
#define __LOGCAT_H

#include "command.h"

class LogCat : public LogScannerCommand {
public:
    void usage();
    void run();
    void setupOptions();
};

#endif // __LOGCAT_H
