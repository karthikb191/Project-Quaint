#ifndef _H_QUAINT_LOGGER
#define _H_QUAINT_LOGGER

#include <Private/LoggerBase.h>
#include <LoggerModule.h>

//namespace Quaint
//{

    //Init Logger must be called before registering Sub-Loggers
    #define DECLARE_LOG_CATEGORY(name)\
    class Logger_##name : public Quaint::LoggerBase {\
        public:\
        Logger_##name() : Quaint::LoggerBase(#name) {}\
        static Logger_##name& get();\
    }

    /*Effective C++ Item-4. Converted this to local-static. object is created on first usage*/
    #define DEFINE_LOG_CATEGORY(name) Logger_##name& Logger_##name::get()\
    { \
        static Logger_##name logger;\
        return logger;\
    }
    
    #define QLOG(name, category, message)\
        Logger_##name::get().log(category, message);
    #define QLOG_V(name, message)\
        Logger_##name::get().logVerbose(message);
    #define QLOG_I(name, message)\
        Logger_##name::get().logInfo(message);
    #define QLOG_W(name, message)\
        Logger_##name::get().logWarning(message);
    #define QLOG_E(name, message)\
        Logger_##name::get().logError(message);

    //#define QLOGGER_NAME(name) logger_##name.getLoggerName();

//}
#endif //_H_QUAINT_LOGGER