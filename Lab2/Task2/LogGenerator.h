#ifndef SISPROG4SEM_LOGGENERATOR_H
#define SISPROG4SEM_LOGGENERATOR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <memory>
#include <atomic>
#include <random>
#include <vector>
#include <sstream>
#include "../Task1/Logger.h"
#include "ThreadSafeQueue.h"

struct tcp_traffic_pkg {
    in_port_t src_port;
    in_addr_t dst_addr;
    in_port_t dst_port;
    size_t size;
};

struct tcp_traffic {
    in_addr_t src_addr;
    std::vector<tcp_traffic_pkg> pkgs;
};

class LogsGenerator {
public:
    explicit LogsGenerator(ThreadSafeQueue<std::string>& q, std::shared_ptr<Logger> logger);
    ~LogsGenerator();
    void stop();

    static std::string ip_to_string(in_addr_t ip);
    static void* generate_sample_traffic(void* arg);

private:
    tcp_traffic generate_traffic_burst();

    std::shared_ptr<Logger> internal_logger;
    ThreadSafeQueue<std::string>& queue;
    std::atomic<bool> stop_{false};
    pthread_t thread_id = 0;

    std::mutex rng_mutex;
    std::random_device rd;
    std::mt19937 gen{rd()};

    std::uniform_int_distribution<in_addr_t> ip_dist{0x0A000001, 0x0A0000FF};
    std::uniform_int_distribution<in_port_t> port_dist{1024, 65535};
    std::uniform_int_distribution<size_t> size_dist{64, 4096};
    std::uniform_int_distribution<int> action_dist{0, 1};
    std::uniform_int_distribution<int> error_dist{0, 100};

    Logger* get_logger() const {
        return internal_logger.get();
    }
};

#endif //SISPROG4SEM_LOGGENERATOR_H