#include "../../Include/Bolt.h"

namespace Bolt
{
    std::unique_ptr<BoltRenderer> BoltRenderer::m_renderer = nullptr;
    BoltRenderer::~BoltRenderer()
    {
        std::cout << "Renderer destroyed!!!!" << "\n";
    }

    void BoltRenderer::startEngine()
    {
        std::cout << "Renderer Started!!!" << "\n";
        m_engineRunning = true;
        //TODO:
    }

    void BoltRenderer::shutdown()
    {
        std::cout << "Renderer shutdown!!!!" << "\n";
        m_engineRunning = false;
        //TODO
    }
}