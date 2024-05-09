#include <ShaderTree/Test_CashMe.h>

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v)            \
    if (vTest == std::string(#v)) \
    return v()

#define IfTestCollectionExist(v, str)        \
    if (vTest.find(#v) != std::string::npos) \
    return v(str)

bool Test_CashMe(const std::string& vTest) {
//    IfTestExist(Test_CashMe_0);
    return true;
}