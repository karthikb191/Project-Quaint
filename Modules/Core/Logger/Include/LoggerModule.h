#ifndef _H_LOGGER_MODULE
#define _H_LOGGER_MODULE
#include<set>
#include<string>
#include<memory>
namespace Quaint
{
    class LoggerModule
    {
        public:
        LoggerModule() {}
        static bool init();
        static bool shutdown();
        static LoggerModule* get();
        bool shouldPrintLogsInCategory(const char* loggerName);
        bool isRunning() { return mRunning; }

        private:
        LoggerModule(const LoggerModule&) = delete;
        LoggerModule(const LoggerModule&&) = delete;

        //TODO: Remove this. Use Singleton<> instead if needed
        static std::unique_ptr<LoggerModule>        mLoggerModule;
        static std::set<std::string>                mRegisteredLogs; 
        static bool                                 mRunning;
        //TODO: Construct Delegates in utils library and add it here for Loggers to subscribe
    };
}
#endif //_H_LOGGER_MODULE