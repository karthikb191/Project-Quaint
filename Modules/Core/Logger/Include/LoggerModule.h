#ifndef _H_LOGGER_MODULE
#define _H_LOGGER_MODULE
#include<set>
#include<string>
#include<memory>
#include<Module.h>

namespace Quaint
{
    class LoggerModule : public Module<LoggerModule>
    {
        BEFRIEND_MODULE(LoggerModule);
    public:
        bool shouldPrintLogsInCategory(const char* loggerName);

    private:
        LoggerModule() = default;
        virtual ~LoggerModule() = default;
        static std::set<std::string>                mRegisteredLogs;
        //TODO: Construct Delegates in utils library and add it here for Loggers to subscribe
    };
}
#endif //_H_LOGGER_MODULE