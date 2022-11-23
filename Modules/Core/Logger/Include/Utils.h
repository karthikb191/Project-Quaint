#include "LogEnums.h"
namespace Quaint
{
    //TODO: Check if this returns a dangling pointer
    const char* logCategoryToString(Category category)
    {
        switch(category)
        {
            case Category::VeryVerbose:
                return "VVerbose";
            case Category::Verbose:
                return "Verbose";
            case Category::Info:
                return "Info";
            case Category::Warn:
                return "Warn";
            case Category::Error:
                return "Error";
            case Category::Critical:
                return "Critical";
            default:
                return "Default";
        }
    }
}