#include <iostream>
#include <QuaintLogger.h>

using namespace Quaint;

#define TempLogger
#define VerboseLogger
#define InfoLogger
#define WarnLogger
#define ErrorLogger

INIT_LOGGER_MODULE

DECLARE_LOG_CATEGORY(TempLogger);
DEFINE_LOG_CATEGORY(TempLogger);

DECLARE_LOG_CATEGORY(VerboseLogger);
DEFINE_LOG_CATEGORY(VerboseLogger);

DECLARE_LOG_CATEGORY(InfoLogger);
DEFINE_LOG_CATEGORY(InfoLogger);

DECLARE_LOG_CATEGORY(WarnLogger);
DEFINE_LOG_CATEGORY(WarnLogger);

DECLARE_LOG_CATEGORY(ErrorLogger);
DEFINE_LOG_CATEGORY(ErrorLogger);

int main()
{
    std::cout << "Hello Logger" << std::endl;

    QLOG(TempLogger, Category::VeryVerbose, "VVerbose Test");

    QLOG_V(VerboseLogger, "Verbose Logger Test");
    QLOG_I(InfoLogger, "Info Logger Test");
    QLOG_W(WarnLogger, "Warn Logger Test");
    QLOG_E(ErrorLogger, "Error Logger Test");


    SHUTDOWN_LOGGER_MODULE
    return 0;
}