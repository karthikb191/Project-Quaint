#ifndef _H_VIDEO_MODULE
#define _H_VIDEO_MODULE

#include <MemoryModule.h>
#include <Module.h>

namespace Quaint { namespace Media {
    class VideoModule : public Module<VideoModule>
    {
        BEFRIEND_MODULE(VideoModule);
    
    public:
        MemoryContext* getVideoMemoryContext() { return m_context; }

    protected:
        void initModule_impl() override
        {
            Module<VideoModule>::initModule_impl();
            //TODO: Modify this to retrieve by name
            m_context = MemoryModule::get().getMemoryManager().getMemoryContenxtByIndex(MEDIA_MEMORY_INDEX);
        }

        void shutdown_impl() override
        {
            Module<VideoModule>::shutdown_impl();
        }
    
    private:
        MemoryContext*      m_context;
    };
}}

#endif //_H_VIDEO_MODULE