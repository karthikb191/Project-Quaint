#ifndef _H_Q_SET
#define _H_Q_SET
#include <QRBTree.h>
#include <Interface/IMemoryContext.h>

namespace Quaint
{
    /*Contains unique elements*/
    template<typename Key, typename Data>
    class QSet
    {
    public:
        QSet(IMemoryContext* context)
        : m_context(context)
        {}

        //TODO: Add Set operations

    private:
        IMemoryContext*     m_context = nullptr;      
        QRBTree             m_tree;
    };
}

#endif