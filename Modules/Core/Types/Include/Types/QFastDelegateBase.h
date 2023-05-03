/*
* DO NOT include this file.
* This is designed to be included with multiple macros defined multiple times
*/
class IMemoryContext;

template <RETURN_TYPE SEPARATOR PARAM_TYPES>
class DELEGATE_NAME
{
    typedef RETURN_TYPE (*Thunk)(const QClosure& SEPARATOR PARAM_TYPES);

public:
    DELEGATE_NAME(IMemoryContext* context) : m_closure(context){} 
    //Binds Member Functions
    template<typename T, typename F>
    void Bind(T* t, RETURN_TYPE(F::*func)(PARAM_TYPES))
    {
        m_closure.Bind(t, func, sizeof(func));
        thunk = &thunkFunc<T, RETURN_TYPE(F::*)(PARAM_TYPES), RETURN_TYPE SEPARATOR PARAM_TYPES>;
    }
    //Binds const member functions
    template<typename T, typename F>
    void Bind(T* t, RETURN_TYPE(F::*func)(PARAM_TYPES) const)
    {
        m_closure.Bind(t, func, sizeof(func));
        thunk = &thunkFunc<T, RETURN_TYPE(F::*)(PARAM_TYPES) const, RETURN_TYPE SEPARATOR PARAM_TYPES>;
    }

    template<typename LAMBDA>
    void BindLambda(LAMBDA lambda)
    {
        m_closure.BindLambda(lambda);
        thunk = &thunkFunc<LAMBDA, RETURN_TYPE(LAMBDA::*)(PARAM_TYPES) const, RETURN_TYPE SEPARATOR PARAM_TYPES>;
    }

    void Bind(RETURN_TYPE(*func)(PARAM_TYPES))
    {
        m_closure.Bind(func, sizeof(func));
        thunk = &thunkFunc<RETURN_TYPE(*)(PARAM_TYPES)>;
    }
    
    //Invoke functions
    RETURN_TYPE invoke(PARAM_ARGUMENTS)
    {
        return thunk(m_closure, PARAM_VALUES);
    }
    RETURN_TYPE operator ()(PARAM_ARGUMENTS)
    {
        return invoke(PARAM_VALUES);
    }

private:
    template<typename T, typename FUNCTYPE, RETURN_TYPE SEPARATOR PARAM_TYPES>
    static RETURN_TYPE thunkFunc(const QClosure& closure SEPARATOR PARAM_ARGUMENTS)
    {
        // Horrible Cast
        // We have pointer to a generic member function here. 
        //union 
        //{
        //    GenericMemberFuncType genericMemFunc;
        //    char memFuncPtrHolder[sizeof(FUNCTYPE)];
        //};
        //genericMemFunc = closure.genericMemFunc;

        // It's not possible to convert member function ptr to void* directly.
        // We therefore bypass this using power of unions!!!  
        //FUNCTYPE func (*(FUNCTYPE*) (void*)(memFuncPtrHolder));
        FUNCTYPE func = reinterpret_cast<FUNCTYPE> (closure.m_genericMemFunc);
        T* obj = reinterpret_cast<T*>(closure.m_obj);
        return (obj->*func)(PARAM_VALUES);
    }

    template<typename FUNCTYPE>
    static RETURN_TYPE thunkFunc(const QClosure& closure SEPARATOR PARAM_ARGUMENTS)
    {
        FUNCTYPE func = reinterpret_cast<FUNCTYPE>(closure.m_genericFuncPtr);
        return (*func)(PARAM_VALUES);
    }

    QClosure m_closure;
    Thunk thunk;
};

template<RETURN_TYPE SEPARATOR PARAM_TYPES, typename X, typename Y>
DELEGATE_NAME<RETURN_TYPE SEPARATOR PARAM_TYPES> HELPER_NAME(const X* x, RETURN_TYPE(Y::*funcToBind)(PARAM_TYPES), IMemoryContext* context)
{
    X* nonConstX = const_cast<X*>(x);
    Y* nonConstY = static_cast<Y*>(nonConstX);
    DELEGATE_NAME<RETURN_TYPE SEPARATOR PARAM_TYPES> delegate(context);
    delegate.Bind(nonConstY, funcToBind);
    return delegate;
}
template<RETURN_TYPE SEPARATOR PARAM_TYPES, typename X, typename Y>
DELEGATE_NAME<RETURN_TYPE SEPARATOR PARAM_TYPES> HELPER_NAME(const X* x, RETURN_TYPE(Y::*funcToBind)(PARAM_TYPES) const, IMemoryContext* context)
{
    X* nonConstX = const_cast<X*>(x);
    Y* nonConstY = static_cast<Y*>(nonConstX);
    DELEGATE_NAME<RETURN_TYPE SEPARATOR PARAM_TYPES> delegate(context);
    delegate.Bind(nonConstY, funcToBind);
    return delegate;
}
template<RETURN_TYPE SEPARATOR PARAM_TYPES>
DELEGATE_NAME<RETURN_TYPE SEPARATOR PARAM_TYPES> HELPER_NAME(RETURN_TYPE(*funcToBind)(PARAM_TYPES), IMemoryContext* context)
{
    DELEGATE_NAME<RETURN_TYPE SEPARATOR PARAM_TYPES> delegate(context);
    delegate.Bind(funcToBind);
    return delegate;
}
