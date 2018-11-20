#ifndef __COMMAND_H
#define __COMMAND_H


#include "sm_base.h"

#include "basethread.h"
#include "handler.h"
#include "scanner.h"
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

namespace po = boost::program_options;

class Command : public basethread_t {
public:
    virtual void run() = 0;
    virtual void setupOptions() = 0;

    static Command* parse(int argc, char** argv);
    static void init();

    po::options_description& getOptions() { return options; }
    void setCommandString(string s) { commandString = s; }
    void setOptionValues(po::variables_map& vm) { optionValues = vm; }

    /// Options passed down to the storage manager
    static void setupSMOptions(po::options_description& options);
    static void setSMOptions(sm_options& sm_opt, const po::variables_map& values);

protected:
    po::options_description options;
    po::variables_map optionValues;

    string commandString;

    /// Options common to all commands (e.g., help and config)
    void setupCommonOptions();

    void helpOption();

private:
    typedef map<string, Command*(*)()> ConstructorMap;
    static ConstructorMap constructorMap;

    static void showCommands();

    template<typename T> static auto check_range(const T& min, const T& max, const std::string& opt_name) {
        return [min, max, opt_name](const T& value) {
            if (value < min || value > max) {
                throw po::validation_error(po::validation_error::invalid_option_value, opt_name, std::to_string(value));
            }
        };
    }
};

class LogScannerCommand : public Command {
public:
    static size_t BLOCK_SIZE;

    virtual void setupOptions();
protected:
    BaseScanner* getScanner(bitset<logrec_t::t_max_logrec>* filter = nullptr);
    BaseScanner* getMergeScanner();
    BaseScanner* getLogArchiveScanner();

    string logdir;
    bool isArchive;
    bool merge;
    string filename;
    int level;
    PageID scan_pid;

private:
    size_t limit;
};

#endif // __COMMAND_H
