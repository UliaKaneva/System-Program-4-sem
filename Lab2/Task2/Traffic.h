#ifndef SISPROG4SEM_TRAFFIC_H
#define SISPROG4SEM_TRAFFIC_H

#include "LogGenerator.h"
#include "LogAnalyzer.h"
#include <vector>
#include <functional>

class Traffic {
    ThreadSafeQueue<std::string> q;
    size_t count_generate_threads;
    size_t count_analyze_threads;
    std::vector<std::unique_ptr<LogsGenerator>> generators;
    std::vector<std::unique_ptr<LogsAnalyzer>> analyzers;
    std::mutex mutex;

    void merge_stats(std::map<in_addr_t, Stats>& result, const std::map<in_addr_t, Stats>& src);

public:
    explicit Traffic(size_t count_generate_threads = 3, size_t count_analyze_threads = 3);

    static void *start_traffic(void *arg);
    static void *analyze_traffic(void *arg);
    static void *dialog(void *arg);

    [[nodiscard]] std::map<in_addr_t, Stats> get_info();
    [[nodiscard]] std::map<in_addr_t, Stats> get_info(in_addr_t addr);
    static void print_and_delete_info(const std::map<in_addr_t, Stats> &info);
    static void print_and_delete_ip(const std::pair<in_addr_t, Stats> &el);
};


#endif //SISPROG4SEM_TRAFFIC_H
