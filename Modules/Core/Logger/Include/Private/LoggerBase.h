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
        void log(Category category, const char* message);
        void logVerbose(const char* message);
        void logInfo(const char* message);
        void logWarning(const char* message);
        void logError(const char* message);

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