#ifndef _H_LOGGER_BASE
#define _H_LOGGER_BASE
#include <LogEnums.h>
#include <string>
namespace Quaint
{
    //TODO: Create a singleton class in Utils library and inherit from it
    //TODO: Inhert from a Module class in Utils
    class LoggerBase
    {
        public:
        LoggerBase(const char* loggerName);

        //void log(Category category, const char* message, ...);
        void log(Category category, const std::string& message);
        void logVerbose(const std::string& message);
        void logInfo(const std::string& message);
        void logWarning(const std::string& message);
        void logError(const std::string& message);

        const char* getLoggerName() {return mLoggerName;}
        private:
        LoggerBase(const LoggerBase&) = delete;
        LoggerBase(const LoggerBase&&) = delete;

        const char* mLoggerName;
        
        protected:
        bool      mEnabled;
    };
}

#endif //_H_LOGGER_BASE