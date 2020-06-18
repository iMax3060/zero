#ifndef __ADDBACKUP_H
#define __ADDBACKUP_H

#include "command.h"

class AddBackup : public Command {
public:
    void setupOptions();

    void run();

private:
    string logdir;

    string backupPath;

    string lsnString;
};

#endif // __ADDBACKUP_H
