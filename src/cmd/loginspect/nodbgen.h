#ifndef __NODBGEN_H
#define __NODBGEN_H

#include "command.h"

class NoDBGen : public Command
{
public:
    void setupOptions();
    void run();
protected:
    void handlePage(fixable_page_h& p);
private:
    string dbfile;
    string logdir;
};

#endif // __NODBGEN_H
