#ifndef _H_QUAINT_LOGGER
#define _H_QUAINT_LOGGER

#include <Private/LoggerBase.h>
#include <LoggerModule.h>

namespace Quaint
{

    //Init Logger must be called before registering Sub-Loggers
    #define DECLARE_LOG_CATEGORY(name)\
    class Logger_##name : public LoggerBase {\
        public:\
        Logger_##name() : LoggerBase(#name) {}\
    }

    /*Effective C++ Item-4*/
    #define DEFINE_LOG_CATEGORY(name) Logger_##name& getLogger##name()\
    { \
        static Logger_##name logger;\
        return logger;\
    }
    
    #define QLOG(name, category, message)\
        getLogger##name().log(category, message);
    #define QLOG_V(name, message)\
        getLogger##name().logVerbose(message);
    #define QLOG_I(name, message)\
        getLogger##name().logInfo(message);
    #define QLOG_W(name, message)\
        getLogger##name().logWarning(message);
    #define QLOG_E(name, message)\
        getLogger##name().logError(message);

    #define QLOGGER_NAME(name) logger_##name.getLoggerName();

}
#endif //_H_QUAINT_LOGGER