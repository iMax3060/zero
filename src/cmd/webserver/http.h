#ifndef __HTTP_H
#define __HTTP_H

#include "command.h"
#include "kits_cmd.h"
#include "agglog.h"
#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>

using namespace boost;
using namespace boost::system;
using namespace boost::asio;

class HandleKits
{
private:
    KitsCommand *kits;
    std::vector<sm_stats_t> stats;
    std::vector<sm_stats_t> stats_delta;
    std::unique_ptr<std::thread> statsThread;
    std::mutex stats_mutex;

public:
    HandleKits();
    int runKits(std::stringstream &options);
    void computeStats();
    void crash();
    void mediaFailure();
    void singlePageFailure();
    std::string getStats(bool);
    std::string isRunning();
    std::string redoProgress();
    std::string undoProgress();
    std::string logAnalysisProgress();
    std::string mediaRecoveryProgress();

    size_t getRedoPagesDirty();
    size_t getRedoPagesTotal();
};

class http_headers
{
private:
   std::string method;
   std::string url;
   std::string version;
   std::map<std::string, std::string> headers;

public:
    std::stringstream options;
    http_headers();
    std::string get_response(HandleKits* kits);
    int content_length();
    void on_read_header(std::string line);
    void on_read_request_line(std::string line);
};


class session {
   asio::streambuf buff;
   http_headers headers;

   static void read_body(std::shared_ptr<session> pThis, HandleKits* kits);

   static void read_next_line(std::shared_ptr<session> pThis, HandleKits* kits);

   static void read_first_line(std::shared_ptr<session> pThis, HandleKits* kits);
public:

   ip::tcp::socket socket;

   session(io_service& io_service)
      :socket(io_service)
   {
   }

   static void interact(std::shared_ptr<session> pThis, HandleKits* kits);

};

#endif // __HTTP_H
