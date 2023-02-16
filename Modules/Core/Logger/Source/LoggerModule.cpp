#include<LoggerModule.h>

namespace Quaint
{
    std::set<std::string> LoggerModule::mRegisteredLogs;

    bool LoggerModule::shouldPrintLogsInCategory(const char* loggerName)
    {
        if(!isInitialized())
            return false;
        
        //TODO: Implement proper logic here
        return true;
    }
}