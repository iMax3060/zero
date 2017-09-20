#ifndef __MERGERUNS_H
#define __MERGERUNS_H

#include "command.h"

class MergeRuns : public Command
{
public:
    void setupOptions();
    void run();
private:
    string indir;
    string outdir;
    size_t level;
    size_t fanin;
    size_t bucketSize;
    size_t replFactor;
};

#endif // __MERGERUNS_H
