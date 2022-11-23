#ifndef _H_QUAINT_LOGGER
#define _H_QUAINT_LOGGER

#include <Private/LoggerBase.h>
#include <LoggerModule.h>

namespace Quaint
{
    //TODO: Make this Generic enough to handle other init/shutdown of other modules
    #define INIT_LOGGER_MODULE      bool b = LoggerModule::get()->init();
    #define SHUTDOWN_LOGGER_MODULE  bool b = LoggerModule::get()->shutdown();

    //Init Logger must be called before registering Sub-Loggers
    #define DECLARE_LOG_CATEGORY(name)\
    class Logger_##name : public LoggerBase {\
        public:\
        Logger_##name() : LoggerBase(#name){};\
    }

    #define DEFINE_LOG_CATEGORY(name) Logger_##name logger##name;
    
    #define QLOG(name, category, message)\
        logger##name.log(category, message);
    #define QLOG_V(name, message)\
        logger##name.logVerbose(message);
    #define QLOG_I(name, message)\
        logger##name.logInfo(message);
    #define QLOG_W(name, message)\
        logger##name.logWarning(message);
    #define QLOG_E(name, message)\
        logger##name.logError(message);

    #define QLOGGER_NAME(name) logger_##name.getLoggerName();

}
#endif //_H_QUAINT_LOGGER