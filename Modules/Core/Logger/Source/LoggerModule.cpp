#include<LoggerModule.h>

namespace Quaint
{
    std::unique_ptr<LoggerModule>        LoggerModule::mLoggerModule;
    
    LoggerModule::LoggerModule()
    {}
    bool LoggerModule::init()
    {
        mRunning = true;
        //TODO:
        return true;
    }
    bool LoggerModule::shutdown()
    {
        mRegisteredLogs.clear();
        mLoggerModule.release();
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