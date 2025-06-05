#include "LogGenerator.h"
#include <chrono>
#include <stdexcept>
#include <unistd.h>


LogsGenerator::LogsGenerator(ThreadSafeQueue<std::string> &q, std::shared_ptr<Logger> logger)
        : queue(q), internal_logger(logger) {}

LogsGenerator::~LogsGenerator() {
    stop();
    if (thread_id) {
        pthread_join(thread_id, nullptr);
    }
}



void LogsGenerator::stop() {
    stop_ = true;
}

std::string LogsGenerator::ip_to_string(in_addr_t ip) {
    struct in_addr addr{};
    addr.s_addr = ip;
    return inet_ntoa(addr);
}

tcp_traffic LogsGenerator::generate_traffic_burst() {
    std::lock_guard<std::mutex> lock(rng_mutex);

    tcp_traffic traffic;
    traffic.src_addr = ip_dist(gen);
    traffic.pkgs.resize(2 + gen() % 10);

    for (auto &pkg: traffic.pkgs) {
        pkg.src_port = port_dist(gen);
        do {
            pkg.dst_addr = ip_dist(gen);
        } while (pkg.dst_addr == traffic.src_addr);

        pkg.dst_port = port_dist(gen);
        pkg.size = size_dist(gen);
    }

    return traffic;
}

void *LogsGenerator::generate_sample_traffic(void *arg) {
    auto *instance = static_cast<LogsGenerator *>(arg);
    if (!instance) return nullptr;

    while (!instance->stop_) {
        auto traffic = instance->generate_traffic_burst();
        Logger *logger = instance->get_logger();

        for (size_t i = 0; i < traffic.pkgs.size(); ++i) {
            if (instance->stop_) break;

            const auto &pkg = traffic.pkgs[i];
            std::ostringstream oss;

            // Форматирование соединения
            oss << ip_to_string(traffic.src_addr) << ":" << ntohs(pkg.src_port)
                << " → " << ip_to_string(pkg.dst_addr) << ":" << ntohs(pkg.dst_port);
            std::string connection_str = oss.str();

            // Обратный путь
            oss.str("");
            oss << ip_to_string(pkg.dst_addr) << ":" << ntohs(pkg.dst_port)
                << " ← " << ip_to_string(traffic.src_addr) << ":" << ntohs(pkg.src_port);
            std::string reverse_str = oss.str();

            // Логирование событий
            if (i == 0) {
                logger->info("CONNECT " + connection_str);
            } else if (i == traffic.pkgs.size() - 1) {
                logger->info("DISCONNECT " + connection_str);
            } else {
                {// Случайные ошибки (1% вероятность)
                    std::lock_guard<std::mutex> lock(instance->rng_mutex);
                    if (instance->error_dist(instance->gen) == 0) {
                        logger->error("DISCONNECT " + connection_str);
                        break;
                    }
                }

                // Случайное действие
                std::string action;
                {
                    std::lock_guard<std::mutex> lock(instance->rng_mutex);
                    action = (instance->action_dist(instance->gen) == 0)
                             ? "POST " + connection_str
                             : "GET " + reverse_str;
                }

                logger->info(action + " (" + std::to_string(pkg.size) + ")");
            }
            usleep(100000); // 10 ms задержка
        }
    }
    return nullptr;
}