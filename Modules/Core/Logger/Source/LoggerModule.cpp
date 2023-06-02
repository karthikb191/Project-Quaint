#include <LoggerModule.h>
#include <assert.h>

namespace Quaint
{

    void LoggerModule::initModule_impl()
    {
        //TODO: Get the list of enabled loggers from an XML file
    }

    bool LoggerModule::shouldPrintLogsInCategory(const char* loggerName)
    {
        if(!isInitialized())
            return false;
        
        //TODO: Implement proper logic here
        return true;
    }
}