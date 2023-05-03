#ifndef _H_Q_FAST_DELEGATES
#define _H_Q_FAST_DELEGATES

#include <cstdlib>
#include <Interface/IMemoryContext.h>

namespace Quaint
{
class GenericClass{};

typedef void(GenericClass::*GenericMemberFuncType)();
typedef void(*GenericFunctionPtrType)(); 

class QClosure
{
public:
    QClosure() = default;
    QClosure(IMemoryContext* context) : m_context(context){}
    QClosure(const QClosure& other)
    {
        *this = other;
    }
    ~QClosure()
    {
        if(m_lambdaSize != 0 && m_obj != nullptr)
        {
            if(m_context == nullptr)
            {
                delete m_obj;
            }
            else
            {
                m_context->Free(m_obj);
            }
        }
    }
    QClosure& operator=(const QClosure& other)
    {     
        m_obj = other.m_obj;
        m_genericFuncPtr = other.m_genericFuncPtr;
        m_genericMemFunc = other.m_genericMemFunc;
        m_context = other.m_context;
        if(other.m_lambdaSize != 0)
        {
            if(m_context == nullptr)
            {
                m_obj = malloc(other.m_lambdaSize);
            }
            else
            {
                m_obj = m_context->Alloc(other.m_lambdaSize);
            }
            memcpy(m_obj, other.m_obj, other.m_lambdaSize);
            m_lambdaSize = other.m_lambdaSize;
        }
        return *this;
    }
    QClosure(const QClosure&&) = delete;
    QClosure& operator=(const QClosure&&) = delete;

    //For member functions
    template<typename T, typename FUNCTYPE>
    void Bind(T* t, FUNCTYPE f, size_t size)
    {
        m_obj = reinterpret_cast<void*>(t);
        m_genericMemFunc = reinterpret_cast<GenericMemberFuncType>(f);
    }

    template<typename LAMBDA>
    void BindLambda(LAMBDA lambda)
    {
        //obj = reinterpret_cast<void*>(&lambda);
        m_lambdaSize = sizeof(LAMBDA);
        if(m_context == nullptr)
        {
            m_obj = (void*)new LAMBDA(lambda);
        }
        else
        {
            m_obj = m_context->Alloc(m_lambdaSize);
            memcpy(m_obj, &lambda, m_lambdaSize);
        }
        m_genericMemFunc = reinterpret_cast<GenericMemberFuncType>(&(LAMBDA::operator()));
    }

    //For non-member and static-member functions
    template<typename FUNCTYPE>
    void Bind(FUNCTYPE f, size_t size)
    {
        m_genericFuncPtr = reinterpret_cast<GenericFunctionPtrType> (f);
    }

    size_t                      m_lambdaSize        = 0;
    IMemoryContext*             m_context           = nullptr;
    void*                       m_obj               = nullptr;
    GenericMemberFuncType       m_genericMemFunc    = nullptr;
    GenericFunctionPtrType      m_genericFuncPtr    = nullptr;
};

#define Q_JOIN_MACRO_UTIL(X, Y) Q_DO_JOIN_MACRO(X, Y)
#define Q_DO_JOIN_MACRO(X, Y) X##Y

// Delegate with 0 params
#define NUM_PARAMS 0
#define RETURN_TYPE typename RET
#define SEPARATOR 
#define PARAM_TYPES 
#define PARAM_ARGUMENTS
#define PARAM_VALUES
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS

// Delegate with 1 param
#define NUM_PARAMS 1
#define RETURN_TYPE typename RET
#define SEPARATOR ,
#define PARAM_TYPES typename PARAM1
#define PARAM_ARGUMENTS PARAM1 p1
#define PARAM_VALUES p1
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS

//Delegate with 2 params
#define NUM_PARAMS 2
#define RETURN_TYPE typename RET
#define SEPARATOR ,
#define PARAM_TYPES typename PARAM1, typename PARAM2
#define PARAM_ARGUMENTS PARAM1 p1, PARAM2 p2
#define PARAM_VALUES p1, p2
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS

//Delegate with 3 params
#define NUM_PARAMS 3
#define RETURN_TYPE typename RET
#define SEPARATOR ,
#define PARAM_TYPES typename PARAM1, typename PARAM2, typename PARAM3
#define PARAM_ARGUMENTS PARAM1 p1, PARAM2 p2, PARAM3 p3
#define PARAM_VALUES p1, p2, p3
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS

//Delegate with 4 params
#define NUM_PARAMS 4
#define RETURN_TYPE typename RET
#define SEPARATOR ,
#define PARAM_TYPES typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4
#define PARAM_ARGUMENTS PARAM1 p1, PARAM2 p2, PARAM3 p3, PARAM4 p4
#define PARAM_VALUES p1, p2, p3, p4
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS

//Delegate with 5 params
#define NUM_PARAMS 5
#define RETURN_TYPE typename RET
#define SEPARATOR ,
#define PARAM_TYPES typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4, typename PARAM5
#define PARAM_ARGUMENTS PARAM1 p1, PARAM2 p2, PARAM3 p3, PARAM4 p4, PARAM5 p5
#define PARAM_VALUES p1, p2, p3, p4, p5
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS

//Delegate with 6 params
#define NUM_PARAMS 6
#define RETURN_TYPE typename RET
#define SEPARATOR ,
#define PARAM_TYPES typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4, typename PARAM5, typename PARAM6
#define PARAM_ARGUMENTS PARAM1 p1, PARAM2 p2, PARAM3 p3, PARAM4 p4, PARAM5 p5, PARAM6 p6
#define PARAM_VALUES p1, p2, p3, p4, p5, p6
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS

//Delegate with 7 params
#define NUM_PARAMS 7
#define RETURN_TYPE typename RET
#define SEPARATOR ,
#define PARAM_TYPES typename PARAM1, typename PARAM2, typename PARAM3, typename PARAM4, typename PARAM5, typename PARAM6, typename PARAM7
#define PARAM_ARGUMENTS PARAM1 p1, PARAM2 p2, PARAM3 p3, PARAM4 p4, PARAM5 p5, PARAM6 p6, PARAM7 p7
#define PARAM_VALUES p1, p2, p3, p4, p5, p6, p7
#define DELEGATE_NAME Q_JOIN_MACRO_UTIL(QFastDelegate,NUM_PARAMS)
#define HELPER_NAME Q_JOIN_MACRO_UTIL(CREATE_AND_BIND_DELEAGATE_,NUM_PARAMS)

#include "QFastDelegateBase.h"

#undef HELPER_NAME
#undef DELEGATE_NAME
#undef PARAM_VALUES
#undef PARAM_ARGUMENTS
#undef PARAM_TYPES
#undef SEPARATOR
#undef RETURN_TYPE
#undef NUM_PARAMS


#define DECLARE_DELEGATE_No_Params(NAME, RET) \
            typedef QFastDelegate0<RET> NAME;
#define DECLARE_DELEGATE_One_Param(NAME, RET, PARAM) \
            typedef QFastDelegate1<RET, PARAM> NAME;
#define DECLARE_DELEGATE_Two_Params(NAME, RET, PARAM1, PARAM2)\
            typedef QFastDelegate2<RET, PARAM1, PARAM2> NAME;
#define DECLARE_DELEGATE_Three_Params(NAME, RET, PARAM1, PARAM2, PARAM3) \
            typedef QFastDelegate3<RET, PARAM1, PARAM2, PARAM3> NAME;
#define DECLARE_DELEGATE_Four_Params(NAME, RET, PARAM1, PARAM2, PARAM3, PARAM4) \
            typedef QFastDelegate4<RET, PARAM1, PARAM2, PARAM3, PARAM4> NAME;
#define DECLARE_DELEGATE_Five_Params(NAME, RET, PARAM1, PARAM2, PARAM3, PARAM4, PARAM5) \
            typedef QFastDelegate5<RET, PARAM1, PARAM2, PARAM3, PARAM4, PARAM5> NAME;
#define DECLARE_DELEGATE_Six_Params(NAME, RET, PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6) \
            typedef QFastDelegate6<RET, PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6> NAME;
#define DECLARE_DELEGATE_Seven_Params(NAME, RET, PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7) \
            typedef QFastDelegate7<RET, PARAM1, PARAM2, PARAM3, PARAM4, PARAM5, PARAM6, PARAM7> NAME;

}
#endif //_H_Q_FAST_DELEGATES