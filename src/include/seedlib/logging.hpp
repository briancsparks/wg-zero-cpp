
#ifndef LOGGING_HPP
#define LOGGING_HPP

// include/seedlib/logging.hpp
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "seedlib/logging_config.hpp"
#include <memory>
#include <string>

namespace seedlib {

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void init(const std::string& app_name = "seedlib") {
        try {
            // Create sinks
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%t] %v");

            std::vector<spdlog::sink_ptr> sinks{console_sink};

            #ifdef ENABLE_FILE_LOGGING
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                "logs/" + app_name + ".log",
                1024 * 1024 * 10,  // 10MB max file size
                3                   // Keep 3 rotated files
            );
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] %v");
            sinks.push_back(file_sink);
            #endif

            #ifdef ENABLE_ASYNC_LOGGING
            spdlog::init_thread_pool(8192, 1);
            logger_ = std::make_shared<spdlog::async_logger>(
                app_name,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
            #else
            logger_ = std::make_shared<spdlog::logger>(app_name, sinks.begin(), sinks.end());
            #endif

            #ifdef ENABLE_DEBUG_LOGGING
            logger_->set_level(spdlog::level::debug);
            logger_->flush_on(spdlog::level::debug);
            #else
            logger_->set_level(spdlog::level::info);
            logger_->flush_on(spdlog::level::err);
            #endif

            spdlog::register_logger(logger_);
            spdlog::set_default_logger(logger_);

        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
            throw;
        }
    }

    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        logger_->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        logger_->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        logger_->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        logger_->error(fmt, std::forward<Args>(args)...);
    }

    // Structured logging support
    template<typename... Args>
    void debug(const std::string& event_name, const std::unordered_map<std::string, std::string>& fields,
               fmt::format_string<Args...> fmt, Args&&... args) {
        log_structured(spdlog::level::debug, event_name, fields, fmt, std::forward<Args>(args)...);
    }

    // Log metrics
    void metric(const std::string& metric_name, double value,
                const std::unordered_map<std::string, std::string>& tags = {}) {
        std::string tag_str;
        for (const auto& [key, val] : tags) {
            tag_str += fmt::format("{}={},", key, val);
        }
        if (!tag_str.empty()) {
            tag_str.pop_back(); // Remove trailing comma
        }
        logger_->info("METRIC {} value={} {}", metric_name, value, tag_str);
    }

private:
    Logger() = default;
    std::shared_ptr<spdlog::logger> logger_;

    template<typename... Args>
    void log_structured(spdlog::level::level_enum level, const std::string& event_name,
                       const std::unordered_map<std::string, std::string>& fields,
                       fmt::format_string<Args...> fmt, Args&&... args) {
        std::string field_str;
        for (const auto& [key, value] : fields) {
            field_str += fmt::format("{}={} ", key, value);
        }
        logger_->log(level, "event={} {} {}", event_name, field_str,
                    fmt::format(fmt, std::forward<Args>(args)...));
    }
};

// Convenience macros
#define LOG_DEBUG(...) ::seedlib::Logger::instance().debug(__VA_ARGS__)
#define LOG_INFO(...) ::seedlib::Logger::instance().info(__VA_ARGS__)
#define LOG_WARN(...) ::seedlib::Logger::instance().warn(__VA_ARGS__)
#define LOG_ERROR(...) ::seedlib::Logger::instance().error(__VA_ARGS__)
#define LOG_METRIC(...) ::seedlib::Logger::instance().metric(__VA_ARGS__)

} // namespace seedlib

// Example usage:
#if 0
int main() {
    seedlib::Logger::instance().init("myapp");

    // Basic logging
    LOG_INFO("Application started");
    LOG_DEBUG("Debug information: {}", some_var);

    // Structured logging
    LOG_INFO("user_login", {
        {"user_id", "12345"},
        {"ip", "192.168.1.1"},
        {"session_id", "abc123"}
    }, "User logged in successfully");

    // Metrics
    LOG_METRIC("request_duration_ms", 42.3, {
        {"endpoint", "/api/v1/users"},
        {"method", "GET"}
    });
}
#endif

#endif //LOGGING_HPP
