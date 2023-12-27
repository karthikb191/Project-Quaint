#include <iostream>
#include <BMFStructures.h>
#include <fstream>
#include <QuaintLogger.h>
#include <MemoryModule.h>
#include <VideoModule.h>

#include <Statistics/Arthmetic.h>
#include <Types/QHeap.h>

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
#include <BMFParsers.h>
#include <AVCCodex.h>
int main()
{
    std::cout << "Hello Video module\n";

    //uint32_t val = 0b00010100000000000000000000000000;
    //uint32_t res = parseExpGolombCode(val);

    /*Heap Tests*/
    Quaint::QHeap<int, _HeapType::EMAX> testMinHeap(Quaint::Media::VideoModule::get().getVideoMemoryContext());
    testMinHeap.insert(10);
    testMinHeap.insert(1);
    testMinHeap.insert(0);
    testMinHeap.insert(48);
    testMinHeap.insert(18);
    testMinHeap.insert(60);
    testMinHeap.insert(40);
    testMinHeap.insert(60);
    testMinHeap.insert(190);

    testMinHeap.print();
    std::cout << "\n\n\n";

    testMinHeap.top();
    testMinHeap.top();
    testMinHeap.top();
    testMinHeap.top();
    testMinHeap.top();

    testMinHeap.print();

    /*Arthmetic coding Tests*/
    const char arithCodingInput[] = "Hello! We are now going to encode this piece of string. Hopefully we can retrieve it safely!";
    constexpr uint32_t codeLength = sizeof(arithCodingInput);
    char outBuffer[4096] = {'\0'};
    codec::encode_Arithmetic(arithCodingInput, codeLength, outBuffer, 4096);

    uint8_t bitBuffer[] = 
    {
        0b10000011,
        0b10111110
    };

    uint8_t kthExpGolombBuff[] = 
    {
        0b00010000,
        0b10000000
    };
    Quaint::Media::BitParser expGolombTestParser(Quaint::Media::VideoModule::get().getVideoMemoryContext(), kthExpGolombBuff, 2);

    std::cout << expGolombTestParser.ue_k(2) << "\n";

    Quaint::Media::BitParser parser(Quaint::Media::VideoModule::get().getVideoMemoryContext(), bitBuffer, 2);
    uint32_t res = parser.readBits(1);
    res = parser.readBits(2);
    res = parser.readBits(3);
    res = parser.readBits(5);
    res = parser.readBits(5);


    Media::BMF bmf("D:\\Works\\Project-Quaint\\Data\\Media\\Sample\\EarthRotation.mov");
    if(!bmf.isOpen())
    {
        QLOG_W(MediaLogger, "Quitting! Failed to open file");
    }
    QLOG_I(MediaLogger, "Successfully opened file");

    bmf.parse();

    bmf.seek(1.5f);

    return 0;
}