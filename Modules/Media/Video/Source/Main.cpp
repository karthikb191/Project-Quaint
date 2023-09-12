#include <iostream>
#include <BMFStructures.h>
#include <fstream>
#include <QuaintLogger.h>
#include <MemoryModule.h>
namespace Quaint
{
    CREATE_MODULE(LoggerModule);
    INIT_MODULE(LoggerModule);

    CREATE_MODULE(MemoryModule);
    INIT_MODULE(MemoryModule);
}


DECLARE_LOG_CATEGORY(MediaLogger);
DEFINE_LOG_CATEGORY(MediaLogger);

using namespace Quaint;
int main()
{
    std::cout << "Hello Video module\n";

    std::fstream file("D:\\Works\\Project-Quaint\\Data\\Media\\Sample\\EarthRotation.mov", std::ios::in | std::ios::binary);
    if(!file.is_open())
    {
        QLOG_W(MediaLogger, "Quitting! Failed to open file");
    }
    QLOG_I(MediaLogger, "Successfully opened file");

    Media::BoxHeader header;

    char buf[4];
    file.read(buf, 4);
    uint32_t size = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
    
    file.read(buf, 4);

    file.close();


    return 0;
}