#ifndef _H_BOLT
#define _H_BOLT
#include <memory>
#include <iostream>
namespace Bolt
{
    class BoltRenderer
    {
    public:
        static BoltRenderer* get()
        {
            if(m_renderer == nullptr)
            {
                m_renderer = std::make_unique<BoltRenderer>();
                std::cout << "Constructed Bolt Renderer" << "\n";
            }
            return m_renderer.get();
        }
        ~BoltRenderer();

        void startEngine();
        void shutdown();

    private:
        static std::unique_ptr<BoltRenderer> m_renderer;
        bool m_engineRunning;
    };
}
#endif //_H_BOLT