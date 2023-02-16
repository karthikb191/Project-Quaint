#ifndef _H_MODULE
#define _H_MODULE

namespace Quaint
{
    template<typename T>
    class Module
    {
    public:
        static T& get()
        {
            static T module;
            return module;
        }
        bool isInitialized()
        {
            T& module = get();
            return module.m_initialized;
        }

        static void initModule()
        {
            get().initModule_impl();
        }
        static void shutdown()
        {
            get().shutdown_impl();
        }   

    protected:
        virtual void initModule_impl()
        {
            T& module = get();
            module.m_initialized = true;
        }

        virtual void shutdown_impl()
        {
            T& module = get();
            module.m_initialized = false;
        }

        Module() = default;
        virtual ~Module() = default;
    private:
        Module(const Module&) = delete;
        Module(const Module&&) = delete;
        Module& operator=(const Module&) = delete;
        Module& operator=(const Module&&) = delete;
        bool m_initialized = false;
    };
#define BEFRIEND_MODULE(TYPE) friend typename Module<TYPE>; 


#define CREATE_MODULE(TYPE)\
class TYPE;\
int createModule##TYPE(){\
    TYPE& module = Module<TYPE>::get();\
    return 0;\
}\
int create_##TYPE = createModule##TYPE();

#define INIT_MODULE(TYPE)\
class TYPE;\
int initModule##TYPE(){\
    Module<TYPE>::initModule();\
    return 0;\
}\
int init_##TYPE = initModule##TYPE();

#define SHUTDOWN_MODULE(TYPE)\
    Module<TYPE>::shutdown();

}
#endif