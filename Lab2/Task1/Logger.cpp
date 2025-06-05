#include "Logger.h"
#include <stdexcept>
#include <iomanip>

std::mutex ConsoleOutPut::console_mutex;

FileOutPut::FileOutPut(const std::string &filename, Mode mode) {
    auto open_mode = std::ios::app;
    if (mode == Mode::Truncate) open_mode = std::ios::trunc;
    file.open(filename, open_mode);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open log file: " + filename);
    }

    file << "";
    if (file.fail()) {
        file.close();
        throw std::runtime_error("No write permissions for log file: " + filename);
    }
}

void FileOutPut::write(const std::string &message) {
    if (!file.is_open()) {
        throw std::runtime_error("Attempt to write to closed log file");
    }
    std::lock_guard<std::mutex> lock(file_mutex);
    file << message << std::endl;
    file.flush();
}

FileOutPut::~FileOutPut() {
    if (file.is_open()) {
        file.close();
    }
}

void ConsoleOutPut::write(const std::string &message) {
    std::lock_guard<std::mutex> lock(console_mutex);
    std::cout << message << std::endl;
}

QueueLogOutPut::QueueLogOutPut(ThreadSafeQueue<std::string> &queue) : queue_(queue) {}
void QueueLogOutPut::write(const std::string &message) {
    queue_.push(message);
}

Logger::Logger(LogLevel level, std::vector<std::unique_ptr<ILogOutPut>> output)
        : level_(level), sinks_(std::move(output)) {}

void Logger::log(LogLevel level, const std::string &message) {
    if (level < level_) return;

    std::string levelStr;
    switch (level) {
        case DEBUG:
            levelStr = "DEBUG";
            break;
        case INFO:
            levelStr = "INFO";
            break;
        case WARNING:
            levelStr = "WARNING";
            break;
        case ERROR:
            levelStr = "ERROR";
            break;
        case CRITICAL:
            levelStr = "CRITICAL";
            break;
    }

    std::time_t now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string timeStr = oss.str();

    std::string logMessage = "[" + timeStr + "] [" + levelStr + "] " + message;

    std::lock_guard<std::mutex> lock(loger_mutex);
    for (auto &sink: sinks_) {
        try {
            sink->write(logMessage);
        } catch (const std::exception& e) {
            std::cerr << "Log write failed: " << e.what() << std::endl;
        }
    }
}

void Logger::debug(const std::string &message) { log(LogLevel::DEBUG, message); }

void Logger::info(const std::string &message) { log(LogLevel::INFO, message); }

void Logger::warning(const std::string &message) { log(LogLevel::WARNING, message); }

void Logger::error(const std::string &message) { log(LogLevel::ERROR, message); }

void Logger::critical(const std::string &message) { log(LogLevel::CRITICAL, message); }

LoggerBuilder &LoggerBuilder::setLevel(LogLevel level) {
    level_ = level;
    return *this;
}

LoggerBuilder &LoggerBuilder::addConsoleSink() {
    sinks_.push_back(std::make_unique<ConsoleOutPut>());
    return *this;
}

LoggerBuilder &LoggerBuilder::addFileSink(const std::string &filename, FileOutPut::Mode mode) {
    sinks_.push_back(std::make_unique<FileOutPut>(filename, mode));
    return *this;
}

LoggerBuilder &LoggerBuilder::addQueueSink(ThreadSafeQueue<std::string> &queue) {
    sinks_.push_back(std::make_unique<QueueLogOutPut>(queue));
    return *this;
}

std::unique_ptr<Logger> LoggerBuilder::build() {
    return std::make_unique<Logger>(level_, std::move(sinks_));
}
