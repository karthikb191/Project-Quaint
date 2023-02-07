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
        LoggerModule();
        bool init();
        bool shutdown();
        static LoggerModule* get();
        bool shouldPrintLogsInCategory(const char* loggerName);
        bool isRunning() { return mRunning; }

        private:
        LoggerModule(const LoggerModule&) = delete;
        LoggerModule(const LoggerModule&&) = delete;

        std::set<std::string>                       mRegisteredLogs; 
        static std::unique_ptr<LoggerModule>        mLoggerModule;
        bool                                        mRunning = false;
        //TODO: Construct Delegates in utils library and add it here for Loggers to subscribe
    };
}
#endif //_H_LOGGER_MODULE