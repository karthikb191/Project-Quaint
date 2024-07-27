#include<iostream>
#include <Types/QArray.h>
#include <Types/QCTString.h>

// 1. Initialized static variable
static const int a = 100;
// 2. Un-Initialized static variable
static int unB;

// 3. Normal function with static variable
int* getFunctionStaticValRef()
{
    static int c = 300;
    return &c;
}

//4. Static function with static variable
static int* staticFunctionWithStaticVar()
{
    static int staticValInStaticFunc = 700;
    return &staticValInStaticFunc;
}

//5. Static function with normal variable
//static int* staticFuncWithNormalVar()
//{
//    int normalVar = 800;
//    return &normalVar; // This is just wrong! Please don't do this.
//}


class ClassA
{
public:
    //6. 
    static int m_a;

    //7. 
    int* staticInNonStaticClassFunc()
    {
        static int unInitstaticVar;
        return &unInitstaticVar;
    }
    //8. 
    static int* staticInStaticClassFunc()
    {
        static int staticVar = 900;
        return &staticVar;
    }
};
int ClassA::m_a = 400;

template<typename T>
class TemplateClass
{
public:
    //9. 
    static int m_t_a;

    //10. 
    int* staticInNonStaticTemplateClassFunc()
    {
        static int staticVar = 1000;
        return &staticVar;
    }
    //11. 
    static int* staticInStaticTemplateClassFunc()
    {
        static int staticVar = 1100;
        return &staticVar;
    }
};
int TemplateClass<int>::m_t_a;

int main()
{
    //Static tests
    const int* refA = &a;
    int* refB = &unB;

    int* heapAlloc1 = new int(10);
    int* heapAlloc2 = new int(10);

    int* refC = getFunctionStaticValRef();
    
    int* heapAlloc3 = new int(10);
    int* heapAlloc4 = new int(10);
    
    const int* ref_staticValInStaticFunc = staticFunctionWithStaticVar();

    ClassA* heapObj = new ClassA();
    int* ref_mA = &ClassA::m_a;

    const int* ref_staticInNonStaticClassFunc = heapObj->staticInNonStaticClassFunc();
    const int* ref_staticInStaticClassFunc = ClassA::staticInStaticClassFunc();

    TemplateClass<int>* heapObj2 = new TemplateClass<int>();
    int* ref_t_mA = &TemplateClass<int>::m_t_a;

    const int* ref_t_staticInNonStaticClassFunc = heapObj2->staticInNonStaticTemplateClassFunc();
    const int* ref_t_staticInStaticClassFunc = TemplateClass<int>::staticInStaticTemplateClassFunc();

    return 0;
}