#ifndef _H_MODULE
#define _H_MODULE

/*
* Modules should be initialized before control enters main. 
* They ideally should not contain any non-local static objects that takes up storage on heap!!
*/
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
            get().m_initialized = true;
        }
        static void shutdown()
        {
            get().shutdown_impl();
            get().m_initialized = false;
        }   

    protected:
        virtual void initModule_impl()
        {
        }

        virtual void shutdown_impl()
        {
        }

        virtual ~Module() = default;
        Module() = default;
    private:
        Module(const Module&) = delete;
        Module(const Module&&) = delete;
        Module& operator=(const Module&) = delete;
        Module& operator=(const Module&&) = delete;
        bool m_initialized = false;
    };
}

#define BEFRIEND_MODULE(TYPE) friend typename Quaint::Module<TYPE>; 

#define CREATE_MODULE(TYPE)\
class TYPE;\
int createModule##TYPE(){\
    TYPE& module = Quaint::Module<TYPE>::get();\
    return 0;\
}\
int create_##TYPE = createModule##TYPE();

#define INIT_MODULE(TYPE)\
class TYPE;\
int initModule##TYPE(){\
    Quaint::Module<TYPE>::initModule();\
    return 0;\
}\
int init_##TYPE = initModule##TYPE();

#define SHUTDOWN_MODULE(TYPE)\
    Quaint::Module<TYPE>::shutdown();


#endif