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
        if(!LoggerModule::get()->isRunning())
        {
            LoggerModule::get()->init();
        }

        mEnabled = LoggerModule::get()->shouldPrintLogsInCategory(loggerName);
    }

    void LoggerBase::log(Category category, const std::string& message)
    {
        if(!mEnabled)
            return;

        std::stringstream outputStream;
        outputStream << "[" << mLoggerName << "]:" << logCategoryToString(category) << ": " << message << std::endl;
        std::cout << outputStream.str();
    }
    void LoggerBase::logVerbose(const std::string& message)
    {
        log(Category::Verbose, message);
    }
    void LoggerBase::logInfo(const std::string& message)
    {
        log(Category::Info, message);
    }
    void LoggerBase::logWarning(const std::string& message)
    {
        log(Category::Warn, message);
    }
    void LoggerBase::logError(const std::string& message)
    {
        log(Category::Error, message);        
    }
    
}