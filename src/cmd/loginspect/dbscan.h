#ifndef __DBSCAN_H
#define __DBSCAN_H

#include "command.h"

class DBScan : public Command {
public:
    void setupOptions();

    void run();

protected:
    virtual void handlePage(PageID pid, const generic_page& page, vol_t* vol);

private:
    string dbfile;

    PageID req_pid;
};

#endif // __DBSCAN_H
