#ifndef _H_LOGGER_MODULE
#define _H_LOGGER_MODULE
#include<memory>
#include<Module.h>
#include<Types/QFastArray.h>
#include<Types/QStaticString.h>
#include<Types/QSet.h>

namespace Quaint
{
    /*No heap allocation should take place in this module if memory module is being used*/
    class LoggerModule : public Module<LoggerModule>
    {
        BEFRIEND_MODULE(LoggerModule);
    public:
        bool shouldPrintLogsInCategory(const char* loggerName);

    private:
        LoggerModule() = default;
        virtual ~LoggerModule() = default;

        virtual void initModule_impl() override;

        QFastArray<QName, 100>                mRegisteredLogs;
        //TODO: Construct Delegates in utils library and add it here for Loggers to subscribe
    };
}
#endif //_H_LOGGER_MODULE