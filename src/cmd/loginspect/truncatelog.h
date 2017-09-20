#ifndef __TRUNCATELOG_H
#define __TRUNCATELOG_H

#include "command.h"

class TruncateLog : public Command
{
public:
    void setupOptions();
    void run();
private:
    string logdir;
    size_t partition;
};

#endif // __TRUNCATELOG_H
