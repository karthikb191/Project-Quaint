#include <iostream>
#include <BMFStructures.h>
#include <fstream>
#include <QuaintLogger.h>
#include <MemoryModule.h>
#include <VideoModule.h>

namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);

    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
    namespace Media
    {

        CREATE_MODULE(VideoModule);
        INIT_MODULE(VideoModule);
    }
}


DECLARE_LOG_CATEGORY(MediaLogger);
DEFINE_LOG_CATEGORY(MediaLogger);

using namespace Quaint;

#include <BMF.h>
#include <BMFParser.h>

int main()
{
    std::cout << "Hello Video module\n";

    Media::BMF bmf("D:\\Works\\Project-Quaint\\Data\\Media\\Sample\\EarthRotation.mov");
    if(!bmf.isOpen())
    {
        QLOG_W(MediaLogger, "Quitting! Failed to open file");
    }
    QLOG_I(MediaLogger, "Successfully opened file");

    bmf.parse();

    return 0;
}