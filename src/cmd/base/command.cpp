#include "command.h"

#include "kits_cmd.h"
#include "genarchive.h"
#include "mergeruns.h"
#include "agglog.h"
#include "logcat.h"
#include "verifylog.h"
#include "truncatelog.h"
#include "propstats.h"
#include "logpagestats.h"
#include "loganalysis.h"
#include "dbscan.h"
#include "addbackup.h"
#include "xctlatency.h"
#include "tracerestore.h"
#include "archstats.h"
#include "logrecinfo.h"
#include "nodbgen.h"
#include "log_carray.h"

#include <boost/foreach.hpp>

/*
 * Adapted from
 * http://stackoverflow.com/questions/582331/is-there-a-way-to-instantiate-objects-from-a-string-holding-their-class-name
 */
Command::ConstructorMap Command::constructorMap;

template<typename T>
Command* createCommand() {
    return new T;
}

#define REGISTER_COMMAND(str, cmd) \
{ \
    Command::constructorMap[str] = &createCommand<cmd>; \
}

void Command::init() {
    /*
     * COMMANDS MUST BE REGISTERED HERE AND ONLY HERE
     */
    REGISTER_COMMAND("logcat", LogCat);
    //REGISTER_COMMAND("logreplay", LogReplay);
    REGISTER_COMMAND("genarchive", GenArchive);
    REGISTER_COMMAND("mergeruns", MergeRuns);
    REGISTER_COMMAND("verifylog", VerifyLog);
    REGISTER_COMMAND("truncatelog", TruncateLog);
    REGISTER_COMMAND("dbscan", DBScan);
    REGISTER_COMMAND("nodbgen", NoDBGen);
    REGISTER_COMMAND("addbackup", AddBackup);
    REGISTER_COMMAND("xctlatency", XctLatency);
    REGISTER_COMMAND("agglog", AggLog);
    REGISTER_COMMAND("logpagestats", LogPageStats);
    REGISTER_COMMAND("loganalysis", LogAnalysis);
    REGISTER_COMMAND("kits", KitsCommand);
    REGISTER_COMMAND("propstats", PropStats);
    REGISTER_COMMAND("tracerestore", RestoreTrace);
    REGISTER_COMMAND("logrecinfo", LogrecInfo);
    REGISTER_COMMAND("archstats", ArchStats);
}

void Command::setupCommonOptions() {
    options.add_options()
            ("help,h",
             "Displays help information regarding a specific command")
            ("config,c", po::value<string>()->implicit_value("zapps.conf"),
             "Specify path to a config file");
}

void Command::showCommands() {
    cerr << "Usage: zapps <command> [options] "
         << endl << "Commands:" << endl;
    ConstructorMap::iterator it;
    for (it = constructorMap.begin(); it != constructorMap.end(); it++) {
        // Options common to all commands
        Command* cmd = (it->second)();
        cmd->setupCommonOptions();
        cmd->setupOptions();
        cerr << it->first << endl << cmd->options << endl << endl;
    }
}

Command* Command::parse(int argc, char** argv) {
    if (argc >= 2) {
        string cmdStr = argv[1];
        std::transform(cmdStr.begin(), cmdStr.end(), cmdStr.begin(), ::tolower);
        if (constructorMap.find(cmdStr) != constructorMap.end()) {
            Command* cmd = constructorMap[cmdStr]();
            cmd->setupCommonOptions();
            cmd->setCommandString(cmdStr);
            cmd->setupOptions();

            po::variables_map vm;
            po::store(po::parse_command_line(argc, argv, cmd->getOptions()), vm);
            if (vm.count("config") > 0) {
                string pathToFile = vm["config"].as<string>();
                std::ifstream file;
                file.open(pathToFile.c_str());
                po::store(po::parse_config_file(file, cmd->getOptions(), true), vm);
            }
            if (vm.count("help") > 0) {
                cmd->helpOption();
                return nullptr;
            }

            po::notify(vm);
            cmd->setOptionValues(vm);
            return cmd;
        }
    }

    showCommands();
    return nullptr;
}

void Command::setupSMOptions(po::options_description& options) {
    boost::program_options::options_description smoptions("Storage Manager Options");
    smoptions.add_options()
            ("db-worker-queueloops", po::value<int>()->default_value(10),
             "Specify the number of spins a transaction worker waits for input")
            ("db-cl-batchsz", po::value<int>()->default_value(10)->notifier(check_range<uint8_t>(1, std::numeric_limits<uint8_t>::max(), "db-cl-batchsz")),
             "Specify the batchsize of a client executing transactions")
            ("activation_delay", po::value<uint>()->default_value(0),
             "Delay to start the log archiver after a checkpoint created by the periodic checkpointer")
            /**SM Options**/
            ("sm_logdir", po::value<string>()->default_value("log"),
             "Path to log directory")
            ("sm_dbfile", po::value<string>()->default_value("db"),
             "Path to the file on which to store database pages")
            ("sm_format", po::value<bool>()->default_value(false)->implicit_value(true),
             "Format SM by emptying logdir and truncating DB file")
            ("sm_truncate_log", po::value<bool>()->default_value(false)
                     ->implicit_value(true),
             "Whether to truncate log partitions at SM shutdown")
            // ("sm_truncate_archive", po::value<bool>()->default_value(false)->implicit_value(true),
            //     "Whether to truncate log archive runs at SM shutdown")
            ("sm_log_partition_size", po::value<int>()->default_value(1024),
             "Size of a log partition in MiB")
            ("sm_log_max_partitions", po::value<int>()->default_value(0),
             "Maximum number of partitions maintained in log directory (0=infinite)")
            ("sm_log_delete_old_partitions", po::value<bool>()->default_value(true),
             "Whether to delete old log partitions as cleaner and checkpointer make progress")
            ("sm_group_commit_size", po::value<int>()->default_value(0),
             "Size in bytes of group commit window (higher -> larger log writes)")
            ("sm_group_commit_timeout", po::value<int>()->default_value(0),
             "Max time to wait (in ms) to fill up group commit window")
            ("sm_log_benchmark_start", po::value<bool>()->default_value(false)->implicit_value(true),
             "Whether to generate benchmark_start log record on SM constructor")
            ("sm_page_img_compression", po::value<int>()->default_value(0),
             "Enables before- and after-image compression for every N log bytes (N=0 turns off)")
            ("sm_bufpoolsize", po::value<int>()->default_value(1024),
             "Size of buffer pool in MiB")
            ("sm_chkpt_interval", po::value<int>(),
             "Interval for checkpoint flushes")
            ("sm_chkpt_log_based", po::value<bool>()->implicit_value(true),
             "Take checkpoints decoupled from buffer and transaction manager, using log scans")
            ("sm_chkpt_use_log_archive", po::value<bool>()->implicit_value(true),
             "Checkpoints use archived LSN to compute min_rec_lsn")
            ("sm_chkpt_print_propstats", po::value<bool>(),
             "Print min recl lsn and dirty page count for every chkpt taken")
            ("sm_log_fetch_buf_partitions", po::value<uint>()->default_value(0),
             "Number of partitions to buffer in memory for recovery")
            ("sm_carray_slots", po::value<int>()->default_value(ConsolidationArray::DEFAULT_ACTIVE_SLOT_COUNT),
             "Max number of active slots in the log's Consolidation Array")
            ("sm_vol_cluster_stores", po::value<bool>()->implicit_value(true),
             "Cluster pages of the same store into extents")
            ("sm_vol_log_reads", po::value<bool>()->implicit_value(true),
             "Generate log records for every page read")
            ("sm_vol_log_writes", po::value<bool>()->implicit_value(true),
             "Generate log records for every page write")
            ("sm_vol_simulate_read_latency", po::value<int>()->default_value(0),
             "Simulated IO-latency of page-reads in ns (actually the min of simulated and actual).")
            ("sm_vol_simulate_write_latency", po::value<int>()->default_value(0),
             "Simulated IO-latency of page-writes in ns (actually the min of simulated and actual).")
            ("sm_vol_readonly", po::value<bool>()->implicit_value(true),
             "Volume will be opened in read-only mode and all writes from buffer pool will be ignored (uses write elision and single-page recovery)")
            ("sm_log_o_direct", po::value<bool>()->implicit_value(true),
             "Whether to open log file with O_DIRECT")
            ("sm_arch_o_direct", po::value<bool>()->implicit_value(true),
             "Whether to open log archive files with O_DIRECT")
            ("sm_vol_o_direct", po::value<bool>()->implicit_value(true),
             "Whether to open volume (i.e., db file) with O_DIRECT")
            ("sm_no_db", po::value<bool>()->default_value(false)->implicit_value(true),
             "No-database mode, a.k.a. log-structured mode, a.k.a. extreme write elision: DB file is written and all fetched pages are rebuilt using single-page recovery from scratch")
            ("sm_batch_segment_size", po::value<size_t>(),
             "Size of segments to use during batch restore warmup")
            ("sm_restart_instant", po::value<bool>()->implicit_value(true),
             "Enable instant restart")
            ("sm_restart_log_based_redo", po::value<bool>()->implicit_value(true),
             "Perform non-instant restart with log-based redo instead of page-based")
            ("sm_rawlock_gc_interval_ms", po::value<int>(),
             "Garbage Collection Interval in ms")
            ("sm_rawlock_lockpool_segsize", po::value<int>(),
             "Segment size Lockpool")
            ("sm_rawlock_xctpool_segsize", po::value<int>(),
             "Segment size Transaction Pool")
            ("sm_rawlock_gc_generation_count", po::value<int>(),
             "Garbage collection generation count")
            ("sm_rawlock_gc_init_generation_count", po::value<int>(),
             "Garbage collection initial generation count")
            ("sm_rawlock_lockpool_initseg", po::value<int>(),
             "Lock pool init segment")
            ("sm_rawlock_gc_free_segment_count", po::value<int>(),
             "Garbage Collection Free Segment Count")
            ("sm_rawlock_gc_max_segment_count", po::value<int>(),
             "Garbage Collection Maximum Segment Count")
            ("sm_locktablesize", po::value<int>(),
             "Lock table size")
            ("sm_rawlock_xctpool_initseg", po::value<int>(),
             "Transaction Pool Initialization Segment")
            ("sm_bf_maintain_emlsn", po::value<bool>()->default_value(false)->implicit_value(true),
             "Maintain the EMLSNs")
            ("sm_bf_warmup_hit_ratio", po::value<int>()->notifier(check_range<int>(0, 100, "sm_bf_warmup_hit_ratio")),
             "Hit ratio to be achieved until system is considered warmed up (int from 0 to 100)")
            ("sm_bf_warmup_min_fixes", po::value<unsigned int>(),
             "Only consider warmup hit ratio once this minimum number of fixes has been performed")
            ("sm_cleaner_decoupled", po::value<bool>()->implicit_value(true),
             "Enable/Disable decoupled cleaner")
            ("sm_cleaner_interval", po::value<int>(),
             "Cleaner sleep interval in ms")
            ("sm_cleaner_workspace_size", po::value<int>()->default_value(0),
             "Size of cleaner write buffer (0=1/128 of buffer pool size)")
            ("sm_cleaner_num_candidates", po::value<int>(),
             "Number of candidate frames considered by each cleaner round")
            ("sm_cleaner_policy", po::value<string>(),
             "Policy used by cleaner to select candidates")
            ("sm_cleaner_min_write_size", po::value<int>(),
             "Page cleaner only writes clusters of pages with this minimum size")
            ("sm_cleaner_min_write_ignore_freq", po::value<int>(),
             "Ignore min_write_size every N rounds of cleaning")
            ("sm_async_eviction", po::value<bool>(),
             "Perform eviction in a dedicated thread, while fixing threads wait")
            ("sm_evictioner_interval_millisec", po::value<int>()->default_value(1000),
             "Evictioner sleep interval in ms when asyc eviction is used")
            ("sm_evictioner_batch_ratio_ppm", po::value<uint32_t>()->default_value(10000),
             "Target value of free buffer frames for the evictioner is ppm")
            ("sm_evict_dirty_pages", po::value<bool>()->implicit_value(true),
             "Do not skip dirty pages when performing eviction and write them out if necessary")
            ("sm_bf_evictioner_flush_dirty_pages", po::value<bool>()->default_value(false)->implicit_value(true),
             "Do flush dirty pages when evicting pages")
            ("sm_bf_evictioner_log_evictions", po::value<bool>(),
             "Generate evict_page log records for every page evicted from the buffer pool")
            ("sm_log_page_fetches", po::value<bool>(),
             "Generate fetch_page log records for every page fetched (and recovered) into the buffer pool")
            ("sm_archiver_workspace_size", po::value<int>()->default_value(1600),
             "Size of the log archiver workspace in MiB")
            ("sm_archiver_bucket_size", po::value<int>()->default_value(1),
             "Archiver bucket size")
            ("sm_archiver_merging", po::value<bool>(),
             "Whether to turn on asynchronous merging with log archiver")
            ("sm_archiver_fanin", po::value<int>(),
             "Log archiver merge fan-in")
            ("sm_archiver_replication_factor", po::value<int>(),
             "Replication factor maintained by the log archive run recycler (0 = never delete a run)")
            ("sm_shutdown_clean", po::value<bool>(),
             "Force buffer before shutting down SM")
            ("sm_archiving", po::value<bool>(),
             "Enable/Disable archiving")
            ("sm_statistics", po::value<bool>(),
             "Enable/Disable display of statistics")
            ("sm_ticker_enable", po::value<bool>(),
             "Enable/Disable ticker (currently always enabled)")
            ("sm_ticker_msec", po::value<int>(),
             "Ticker interval in millisec")
            ("sm_ticker_print_tput", po::value<bool>(),
             "Print transaction throughput on every tick to a file tput.txt")
            ("sm_restore_instant", po::value<bool>(),
             "Enable/Disable instant restore")
            ("sm_write_elision", po::value<bool>(),
             "Enable/Disable write elision in buffer pool")
            ("sm_archiver_eager", po::value<bool>(),
             "Enable/Disable eager archiving")
            ("sm_archiver_read_whole_blocks", po::value<bool>(),
             "Enable/Disable reading whole blocks in the archiver")
            ("sm_archiver_slow_log_grace_period", po::value<int>(),
             "Enable/Disable slow log grace period")
            ("sm_archdir", po::value<string>()->default_value("archive"),
             "Path to archive directory");
    options.add(smoptions);
}

void Command::helpOption() {
    cerr << "Usage: zapps Command:" << commandString << " [options] "
         << endl << options << endl;
}

size_t LogScannerCommand::BLOCK_SIZE = 1024 * 1024;

BaseScanner* LogScannerCommand::getScanner(
        bitset<logrec_t::t_max_logrec>* filter) {
    BaseScanner* s;
    if (isArchive) {
        if (merge) {
            s = new MergeScanner(optionValues);
        } else {
            s = new LogArchiveScanner(optionValues);
        }
    } else {
        s = new BlockScanner(optionValues, filter);
    }

    if (!filename.empty()) {
        if (!isArchive) {
            s->setRestrictFile(logdir + "/" + filename);
        } else {
            s->setRestrictFile(filename);
        }
    }

    return s;
}

void LogScannerCommand::setupOptions() {
    setupSMOptions(options);
    po::options_description logscanner("Log Scanner Options");
    logscanner.add_options()
            ("logdir,l", po::value<string>(&logdir)->required(),
             "Directory containing log to be scanned")
            ("file,f", po::value<string>(&filename)->default_value(""),
             "Scan only a specific file inside the given directory")
            ("archive,a", po::value<bool>(&isArchive)->default_value(false)
                     ->implicit_value(true),
             "Scan log archive files isntead of normal recovery log")
            ("merge,m", po::value<bool>(&merge)->default_value(false)
                     ->implicit_value(true),
             "Merge archiver input so that global sort order is produced")
            ("limit,n", po::value<size_t>(&limit)->default_value(0),
             "Number of log records to scan")
            ("level", po::value<int>(&level)->default_value(-1),
             "Level of log archive to scan (-1 for all)")
            ("pid", po::value<PageID>(&scan_pid)->default_value(0),
             "PageID on which to begin scan (archive only)");
    options.add(logscanner);
}

void Command::setSMOptions(sm_options& sm_opt, const po::variables_map& values) {
    for (const po::variables_map::value_type& pair : values) {
        const std::string& key = pair.first;
        try {
            sm_opt.set_int_option(key, values[key].as<int>());
        }
        catch (boost::bad_any_cast const& e) {
            try {
                cerr << "Set option " << key << " to " << values[key].as<bool>() << endl;
                sm_opt.set_bool_option(key, values[key].as<bool>());
            }
            catch (boost::bad_any_cast const& e) {
                try {
                    sm_opt.set_string_option(key, values[key].as<string>());
                }
                catch (boost::bad_any_cast const& e) {
                    try {
                        sm_opt.set_int_option(key, values[key].as<uint>());
                    }
                    catch (boost::bad_any_cast const& e) {
                        cerr << "Could not process option " << key
                             << " .. skippking." << endl;
                        continue;
                    }
                }
            }
        }
    };
}
