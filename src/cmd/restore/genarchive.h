#ifndef __GENARCHIVE_H
#define __GENARCHIVE_H

#include "command.h"

class GenArchive : public Command
{
public:
    void setupOptions();
    void run();
private:
    string logdir;
    string archdir;
    long maxLogSize;
    size_t bucketSize;
};

#endif // __GENARCHIVE_H
