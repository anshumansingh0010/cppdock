#include <common/Logger.hpp>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace cppdock::common {
Logger::Logger() : minLevel(LogLevel::INFO), logToFile(false) {}

Logger::~Logger() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::init(const std::string &filename, LogLevel level,
                  bool writeToFile) {
    std::lock_guard<std::mutex> lock(logMutex);
    minLevel = level;
    logToFile = writeToFile;

    if (logToFile && !logFile.is_open()) {
        try {
            std::filesystem::path fullPath(filename);

            if (fullPath.has_parent_path()) {
                std::filesystem::create_directories(fullPath.parent_path());
            }

            logFile.open(filename, std::ios::app);

            if (!logFile.is_open()) {
                std::cerr << "[Logger Error] Failed to open log file at: "
                          << filename << "\n";
            }
        } catch (const std::filesystem::filesystem_error &e) {
            std::cerr << "[Logger Error] Filesystem creation crash: "
                      << e.what() << "\n";
        }
    }
}

void Logger::log(LogLevel level, const std::string &message) {
    if (level < minLevel)
        return;

    std::lock_guard<std::mutex> lock(logMutex);

    auto now = std::time(nullptr);
    auto localTime = *std::localtime(&now);

    std::ostringstream timeStream;
    timeStream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    std::ostringstream logStream;
    logStream << "[" << timeStream.str() << "] [" << levelToString(level)
              << "] " << message << "\n";

    std::string logOutput = logStream.str();

    if (level >= LogLevel::ERROR) {
        std::cerr << logOutput;
    } else {
        std::cout << logOutput;
    }

    if (logToFile && logFile.is_open()) {
        logFile << logOutput;
        logFile.flush();
    }
}
} // namespace cppdock::common