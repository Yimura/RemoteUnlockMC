#pragma once

namespace RemoteUnlock
{
    class LogHelper
    {
    private:
    public:
        LogHelper();
        ~LogHelper();

        static void ConsoleLogger(LogMessagePtr message);
        static const std::string_view ConsoleLevelToStr(eLogLevel level);
    };
} // namespace RemoteUnlock
