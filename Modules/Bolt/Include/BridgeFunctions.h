#ifndef _H_BRIDGE_FUNCTIONS
#define _H_BRIDGE_FUNCTIONS

/* There need to be a single grahics API level implementation for all these functions */
namespace Bolt
{
    class GraphicsResource;

    void mapBufferResource(IBufferImpl* resource, void** out);
    void unmapBufferResource(IBufferImpl* resource);
}


#endif //_H_BRIDGE_FUNCTIONS