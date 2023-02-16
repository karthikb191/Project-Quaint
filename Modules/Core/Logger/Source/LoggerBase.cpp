#include<Private/LoggerBase.h>
#include<LoggerModule.h>
#include<iostream>
#include<sstream>
#include<Utils.h>
namespace Quaint
{
    LoggerBase::LoggerBase(const char* loggerName)
    : mLoggerName(loggerName)
    , mEnabled(true)
    {
        mEnabled = LoggerModule::get().shouldPrintLogsInCategory(loggerName);
    }

    void LoggerBase::log(Category category, const char* message)
    {
        if(!mEnabled)
            return;

        char buffer[4096];
        sprintf_s(buffer, "[%s]: %s : %s \n", mLoggerName, logCategoryToString(category), message);
        std::cout << buffer;
    }
    void LoggerBase::logVerbose(const char* message)
    {
        log(Category::Verbose, message);
    }
    void LoggerBase::logInfo(const char* message)
    {
        log(Category::Info, message);
    }
    void LoggerBase::logWarning(const char* message)
    {
        log(Category::Warn, message);
    }
    void LoggerBase::logError(const char* message)
    {
        log(Category::Error, message);        
    }
    
}