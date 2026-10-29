#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <common/Utils.hpp>
#include <fstream>
#include <mutex>
#include <string>

namespace cppdock::common {

enum class LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
  public:
    static Logger &getInstance() {
        static Logger instance;
        return instance;
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    void init(const std::string &filename = PROJECT_ROOT_DIR "/state/app.log",
              LogLevel level = LogLevel::INFO, bool writeToFile = true);
    void log(LogLevel level, const std::string &message);

  private:
    explicit Logger();
    ~Logger();
    std::string levelToString(LogLevel level) {
        switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
        }
    }
    LogLevel minLevel;
    bool logToFile;
    std::ofstream logFile;
    std::mutex logMutex;
};

} // namespace cppdock::common

#define LOG_INIT(file, lvl, file_enable) cppdock::common::Logger::getInstance().init(file, lvl, file_enable)
#define LOG_INIT_DEFAULT() cppdock::common::Logger::getInstance().init()
#define LOG_DEBUG(...)    cppdock::common::Logger::getInstance().log(cppdock::common::LogLevel::DEBUG, cppdock::common::formatMsg(__VA_ARGS__))
#define LOG_INFO(...)     cppdock::common::Logger::getInstance().log(cppdock::common::LogLevel::INFO, cppdock::common::formatMsg(__VA_ARGS__))
#define LOG_WARN(...)     cppdock::common::Logger::getInstance().log(cppdock::common::LogLevel::WARNING, cppdock::common::formatMsg(__VA_ARGS__))
#define LOG_ERROR(...)    cppdock::common::Logger::getInstance().log(cppdock::common::LogLevel::ERROR, cppdock::common::formatMsg(__VA_ARGS__))
#define LOG_CRITICAL(...) cppdock::common::Logger::getInstance().log(cppdock::common::LogLevel::CRITICAL, cppdock::common::formatMsg(__VA_ARGS__))

#endif