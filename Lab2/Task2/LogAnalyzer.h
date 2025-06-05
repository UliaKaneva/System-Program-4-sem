#ifndef SISPROG4SEM_LOGANALYZER_H
#define SISPROG4SEM_LOGANALYZER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <mutex>
#include <atomic>
#include <pthread.h>
#include "LogGenerator.h"

struct Stats {
    size_t total_received = 0;
    size_t total_sent = 0;
    std::map<in_addr_t, tcp_traffic> connected;
    std::vector<in_addr_t> disconnected;
};

class LogsAnalyzer {
private:
    ThreadSafeQueue<std::string> &queue;
    std::map<in_addr_t, Stats> info;
    std::mutex info_mutex;
    std::atomic<bool> stop_flag{false};
    pthread_t thread_id = 0;

    void connect(in_addr_t dst_addr, in_port_t dst_port,
                 in_addr_t src_addr, in_port_t src_port);

    void disconnect(in_addr_t dst_addr, in_port_t dst_port,
                    in_addr_t src_addr, in_port_t src_port);

    void get(in_addr_t dst_addr, in_port_t dst_port,
             in_addr_t src_addr, in_port_t src_port,
             size_t size_information);

    void post(in_addr_t dst_addr, in_port_t dst_port,
              in_addr_t src_addr, in_port_t src_port,
              size_t size_information);

public:
    explicit LogsAnalyzer(ThreadSafeQueue<std::string> &queue);
    ~LogsAnalyzer();

    void start();
    void stop();

    std::map<in_addr_t, Stats> get_info();
    std::map<in_addr_t, Stats> get_info(in_addr_t ip);

    static void* analyzing_traffic(void* arg);

    static in_addr_t string_to_ip(const std::string &ip_str);
    static std::vector<std::string> split(const std::string &s, char delimiter);
    static std::string delete_datatime(const std::string &logLine);
};
#endif //SISPROG4SEM_LOGANALYZER_H
