#include "LogHelper.hpp"
#include <filesystem>

namespace RemoteUnlock
{
    LogHelper::LogHelper()
    {
        Logger::Init();
        Logger::AddSink(ConsoleLogger);
    }

    LogHelper::~LogHelper()
    {
        Logger::Destroy();
    }

    void LogHelper::ConsoleLogger(LogMessagePtr message)
    {
        const auto& location = message->Location();
        const auto file      = std::filesystem::path(message->Location().file_name()).filename().string();

        std::cout << "[" << ConsoleLevelToStr(message->Level()) << "/" << file << ":" << location.line() << "] "
                  << message->Message();
    }

    const std::string_view LogHelper::ConsoleLevelToStr(eLogLevel level)
    {
        switch (level)
        {
        case eLogLevel::VERBOSE:
            return "VERB";
        case eLogLevel::INFO:
            return "INFO";
        case eLogLevel::WARNING:
            return "WARNING";
        case eLogLevel::FATAL:
            return "FATAL";
        }
        return "UNKNOWN";
    }
} // namespace RemoteUnlock
