#include<LoggerModule.h>

namespace Quaint
{
    std::unique_ptr<LoggerModule> LoggerModule::mLoggerModule = nullptr;
    std::set<std::string> LoggerModule::mRegisteredLogs;
    bool LoggerModule::mRunning = false;

    bool LoggerModule::init()
    {
        get();
        mRunning = true;
        //TODO:
        return true;
    }
    bool LoggerModule::shutdown()
    {
        mRegisteredLogs.clear();
        mRunning = false;
        return true;
    }
    LoggerModule* LoggerModule::get()
    {
        if(mLoggerModule.get() == nullptr)
            mLoggerModule = std::make_unique<LoggerModule>();
        
        return mLoggerModule.get();
    }
    bool LoggerModule::shouldPrintLogsInCategory(const char* loggerName)
    {
        if(!mRunning)
            return false;
        
        //TODO: Implement proper logic here
        return true;
    }
}