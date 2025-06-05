#include "LogAnalyzer.h"
#include <stdexcept>
#include <unistd.h>

LogsAnalyzer::LogsAnalyzer(ThreadSafeQueue<std::string> &queue) : queue(queue) {}

LogsAnalyzer::~LogsAnalyzer() {
    stop();
    if (thread_id) {
        pthread_join(thread_id, nullptr);
    }
}

void LogsAnalyzer::start() {
    if (thread_id != 0) return;

    stop_flag = false;
    if (pthread_create(&thread_id, nullptr, &LogsAnalyzer::analyzing_traffic, this) != 0) {
        throw std::runtime_error("Failed to create analysis thread");
    }
}

void LogsAnalyzer::stop() {
    stop_flag = true;
}

in_addr_t LogsAnalyzer::string_to_ip(const std::string &ip_str) {
    struct in_addr addr {};
    if (inet_pton(AF_INET, ip_str.c_str(), &addr) != 1) {
        throw std::invalid_argument("Invalid IP address: " + ip_str);
    }
    return addr.s_addr;
}

std::vector<std::string> LogsAnalyzer::split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(s);

    while (std::getline(stream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

void* LogsAnalyzer::analyzing_traffic(void* arg) {
    auto* analyzer = static_cast<LogsAnalyzer*>(arg);
    if (!analyzer) return nullptr;

    while (!analyzer->stop_flag) {
        try {
            std::string log_entry = analyzer->queue.pop();
            if (log_entry.empty()) // Проверка poison pill
                break;
            std::string content = delete_datatime(log_entry);
            std::vector<std::string> parts = split(content, ' ');

            if (parts.size() < 5) {
                continue;
            }

            std::string log_type = parts[1];
            std::vector<std::string> src_parts = split(parts[2], ':');
            std::vector<std::string> dst_parts = split(parts[4], ':');

            if (src_parts.size() != 2 || dst_parts.size() != 2) {
                continue;
            }

            in_addr_t src_addr = string_to_ip(src_parts[0]);
            in_port_t src_port = std::stoul(src_parts[1]);
            in_addr_t dst_addr = string_to_ip(dst_parts[0]);
            in_port_t dst_port = std::stoul(dst_parts[1]);

            if (log_type == "CONNECT") {
                analyzer->connect(dst_addr, dst_port, src_addr, src_port);
            }
            else if (log_type == "GET") {
                size_t size = 0;
                if (parts.size() > 5) {
                    size = std::stoul(parts[5].substr(1, parts[5].size() - 2));
                }
                analyzer->get(dst_addr, dst_port, src_addr, src_port, size);
            }
            else if (log_type == "POST") {
                size_t size = 0;
                if (parts.size() > 5) {
                    size = std::stoul(parts[5].substr(1, parts[5].size() - 2));
                }
                analyzer->post(dst_addr, dst_port, src_addr, src_port, size);
            }
            else if (log_type == "DISCONNECT") {
                analyzer->disconnect(dst_addr, dst_port, src_addr, src_port);
            }
        }
        catch (const std::exception& e) {
            continue;
        }
    }
    return nullptr;
}

void LogsAnalyzer::connect(in_addr_t dst_addr, in_port_t dst_port,
                           in_addr_t src_addr, in_port_t src_port) {
    std::lock_guard<std::mutex> lock(info_mutex);

    auto& dst_stats = info[dst_addr];
    auto& traffic = dst_stats.connected[src_addr];

    if (traffic.pkgs.empty()) {
        traffic.src_addr = src_addr;
        traffic.pkgs.push_back({src_port, dst_addr, dst_port, 0});
    }
}

void LogsAnalyzer::get(in_addr_t dst_addr, in_port_t dst_port,
                       in_addr_t src_addr, in_port_t src_port,
                       size_t size) {
    std::lock_guard<std::mutex> lock(info_mutex);

    info[dst_addr].total_received += size;
    info[src_addr].total_sent += size;

    auto& traffic = info[dst_addr].connected[src_addr];
    traffic.pkgs.push_back({src_port, dst_addr, dst_port, size});
}

void LogsAnalyzer::post(in_addr_t dst_addr, in_port_t dst_port,
                        in_addr_t src_addr, in_port_t src_port,
                        size_t size) {
    get(dst_addr, dst_port, src_addr, src_port, size);
}

void LogsAnalyzer::disconnect(in_addr_t dst_addr, in_port_t dst_port,
                              in_addr_t src_addr, in_port_t src_port) {
    std::lock_guard<std::mutex> lock(info_mutex);

    if (auto it = info.find(dst_addr); it != info.end()) {
        it->second.connected.erase(src_addr);
        it->second.disconnected.push_back(src_addr);  // Добавлено
    }

    if (auto it = info.find(src_addr); it != info.end()) {
        it->second.connected.erase(dst_addr);
        it->second.disconnected.push_back(dst_addr);  // Добавлено
    }
}

std::map<in_addr_t, Stats> LogsAnalyzer::get_info() {
    std::lock_guard<std::mutex> lock(info_mutex);
    return info;
}

std::map<in_addr_t, Stats> LogsAnalyzer::get_info(in_addr_t ip) {
    std::lock_guard<std::mutex> lock(info_mutex);
    return {{ip, info[ip]}};
}

std::string LogsAnalyzer::delete_datatime(const std::string &logLine) {
    if (logLine.empty() || logLine[0] != '[')
        return logLine;

    size_t end_pos = logLine.find(']', 1);
    if (end_pos == std::string::npos)
        return logLine;

    size_t start = end_pos + 1;
    while (start < logLine.size() && std::isspace(logLine[start]))
        start++;

    return start < logLine.size() ? logLine.substr(start) : "";
}