#ifndef __PROPSTATS_H
#define __PROPSTATS_H

#include "command.h"

class PropStats : public LogScannerCommand {
public:
    void run();

    void setupOptions();

private:
    size_t psize;
};

#endif // __PROPSTATS_H
