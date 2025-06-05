#ifndef SISPROG4SEM_LOGGER_H
#define SISPROG4SEM_LOGGER_H

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string>

#include <mutex>
#include "../Task2/ThreadSafeQueue.h"

enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class ILogOutPut {
public:
    virtual void write(const std::string &message) = 0;
    virtual ~ILogOutPut() = default;
};

class ConsoleOutPut : public ILogOutPut {
private:
    static std::mutex console_mutex;  // Добавлено
public:
    void write(const std::string &message) override;
};

class FileOutPut : public ILogOutPut {
public:
    enum class Mode { Append, Truncate };
    explicit FileOutPut(const std::string &filename, Mode mode = Mode::Append); // Значение по умолчанию здесь
    void write(const std::string &message) override;
    ~FileOutPut() override;
private:
    std::ofstream file;
    std::mutex file_mutex;
};

class QueueLogOutPut : public ILogOutPut {
protected:
    ThreadSafeQueue<std::string> &queue_;

public:
    explicit QueueLogOutPut(ThreadSafeQueue<std::string> &q);
    void write(const std::string &str) override;
};

class Logger {
public:
    Logger(LogLevel level, std::vector<std::unique_ptr<ILogOutPut>> output);
    void log(LogLevel level, const std::string &message);
    void debug(const std::string &message);
    void info(const std::string &message);
    void warning(const std::string &message);
    void error(const std::string &message);
    void critical(const std::string &message);
private:
    LogLevel level_;
    std::vector<std::unique_ptr<ILogOutPut>> sinks_;
    std::mutex loger_mutex;
};

class LoggerBuilder {
public:
    LoggerBuilder &setLevel(LogLevel level);
    LoggerBuilder &addConsoleSink();
    LoggerBuilder &addFileSink(const std::string &filename, FileOutPut::Mode mode = FileOutPut::Mode::Append); // Значение по умолчанию только здесь
    LoggerBuilder &addQueueSink(ThreadSafeQueue<std::string> &queue);
    std::unique_ptr<Logger> build();
private:
    LogLevel level_ = LogLevel::INFO;
    std::vector<std::unique_ptr<ILogOutPut>> sinks_;
};

#endif //SISPROG4SEM_LOGGER_H