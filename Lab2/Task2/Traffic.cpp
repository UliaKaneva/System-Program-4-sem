#include "Traffic.h"
#include <vector>

Traffic::Traffic(size_t count_generate_threads, size_t count_analyze_threads)
        : count_analyze_threads(count_analyze_threads), count_generate_threads(count_generate_threads) {
    std::unique_ptr<Logger> l = LoggerBuilder()
            .setLevel(DEBUG)
            .addQueueSink(q)
            .addFileSink("generate.log", FileOutPut::Mode::Truncate)
            .build();
    std::shared_ptr<Logger> loggerr(l.release());
    for (size_t i = 0; i < count_generate_threads; ++i) {
        auto ptr = std::make_unique<LogsGenerator>(q, loggerr);
        generators.push_back(std::move(ptr));
    }
    for (size_t i = 0; i < count_analyze_threads; ++i) {
        auto ptr = std::make_unique<LogsAnalyzer>(q);
        analyzers.push_back(std::move(ptr));
    }
}

void *Traffic::start_traffic(void *arg) {
    auto *traffic = static_cast<Traffic *>(arg);
    if (arg == nullptr) {
        return nullptr;
    }
    std::vector<pthread_t> threads(traffic->count_generate_threads);
    for (size_t i = 0; i < traffic->count_generate_threads; ++i) {
        pthread_create(&threads[i], nullptr, &LogsGenerator::generate_sample_traffic, traffic->generators[i].get());
    }

    for (size_t i = 0; i < traffic->count_generate_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    return nullptr;
}

void *Traffic::analyze_traffic(void *arg) {
    auto *traffic = static_cast<Traffic *>(arg);
    if (arg == nullptr) {
        return nullptr;
    }
    std::vector<pthread_t> threads(traffic->count_analyze_threads + 1);
    for (size_t i = 0; i < traffic->count_analyze_threads; ++i) {
        pthread_create(&threads[i], nullptr, &LogsAnalyzer::analyzing_traffic, traffic->analyzers[i].get());
    }
    pthread_create(&threads[traffic->count_analyze_threads], nullptr, &dialog, traffic);

    for (size_t i = 0; i < traffic->count_analyze_threads + 1; ++i) {
        pthread_join(threads[i], nullptr);
    }
    return nullptr;
}

void Traffic::merge_stats(std::map<in_addr_t, Stats>& result, const std::map<in_addr_t, Stats>& src) {
    for (auto &pair : src) {
        in_addr_t ip = pair.first;
        const Stats &src_stats = pair.second;
        Stats &dst_stats = result[ip];

        dst_stats.total_sent += src_stats.total_sent;
        dst_stats.total_received += src_stats.total_received;
        dst_stats.disconnected.insert(dst_stats.disconnected.end(),
                                      src_stats.disconnected.begin(),
                                      src_stats.disconnected.end());

        for (auto &src_conn : src_stats.connected) {
            in_addr_t src_ip = src_conn.first;
            const tcp_traffic &src_info = src_conn.second;
            tcp_traffic &dst_info = dst_stats.connected[src_ip];
            dst_info.pkgs.insert(dst_info.pkgs.end(), src_info.pkgs.begin(), src_info.pkgs.end());
            dst_info.src_addr = src_info.src_addr;
        }
    }
}

std::map<in_addr_t, Stats> Traffic::get_info() {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<in_addr_t, Stats> result;

    for (auto &analyzer : analyzers) {
        merge_stats(result, analyzer->get_info());
    }

    for (auto &conn : result) {
        for (auto dis : conn.second.disconnected) {
            auto it = conn.second.connected.find(dis);
            if (it != conn.second.connected.end()) {
                conn.second.connected.erase(it);
            }
        }
    }
    return result;
}

std::map<in_addr_t, Stats> Traffic::get_info(in_addr_t addr) {
    std::lock_guard<std::mutex> lock(mutex);
    std::map<in_addr_t, Stats> result;

    for (auto &analyzer : analyzers) {
        merge_stats(result, analyzer->get_info(addr));
    }

    for (auto &conn : result) {
        for (auto dis : conn.second.disconnected) {
            auto it = conn.second.connected.find(dis);
            if (it != conn.second.connected.end()) {
                conn.second.connected.erase(it);
            }
        }
    }
    return result;
}

void Traffic::print_and_delete_info(const std::map<in_addr_t, Stats> &info) {
	for (auto &el : info) {
		print_and_delete_ip(el);
	}
}

void Traffic::print_and_delete_ip(const std::pair<in_addr_t, Stats> &el) {
	std::cout << "------------------\n";
	std::cout << "IP: " << LogsGenerator::ip_to_string(el.first) << std::endl;
	std::cout << "Total sent: " << el.second.total_sent << std::endl;
	std::cout << "Total received: " << el.second.total_received << std::endl;
	std::cout << "Connected ip:\n";
	for (auto &addr : el.second.connected) {
		std::cout << "\t" << LogsGenerator::ip_to_string(addr.second.src_addr) << std::endl;
		std::cout << "\tPackets dst:src\n";
		for (int i = 0; i < addr.second.pkgs.size(); ++i) {
			std::cout << "\t\t" << std::to_string(addr.second.pkgs[i].dst_port) << ":"
			          << std::to_string(addr.second.pkgs[i].src_port) << " " << std::to_string(addr.second.pkgs[i].size)
			          << " B" << std::endl;
		}
	}
}

void *Traffic::dialog(void *arg) {
	auto *a = static_cast<Traffic *>(arg);
	if (a == nullptr) {
		return nullptr;
	}
	std::string line;
	std::cout << "Write: \n1. get all\n2. get <addr>\n3. stop\n";
	while (getline(std::cin, line)) {
        if (line == "stop") {
            for (int i = 0; i < a->count_generate_threads; ++i) {
                a->generators[i]->stop();
            }
            for (int i = 0; i < a->count_analyze_threads; ++i) {
                a->analyzers[i]->stop();
            }
            for (size_t i = 0; i < a->count_analyze_threads; ++i) {
                a->q.push("");
            }
            break;
        } else {
            if (line == "get all") {
                Traffic::print_and_delete_info(a->get_info());
            } else {
                auto vec = LogsAnalyzer::split(line, ' ');
                if (vec.size() == 2 and vec[0] == "get") {
                    in_addr_t t = LogsAnalyzer::string_to_ip(vec[1]);
                    Traffic::print_and_delete_info(a->get_info(t));

                } else {
                    std::cout << "Incorrect command\n";
                }
            }
        }
        std::cout << "Write: \n1. get all\n2. get <addr>\n3. stop\n";
	}
	return nullptr;
}
