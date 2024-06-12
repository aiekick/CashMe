#include <src/Test_Module.h>

#include <Utils/Utils.h>

////////////////////////////////////////////////////////////////////////////
//// parseDescription //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

bool test_parseDescription_0() {
    std::string entity;
    std::string operation;
    std::string description;
    if (!parseDescription("CB TOTO VA AU ZOO 25/03/12", entity, operation, description)) {
        return false;
    }
    if (entity != "TOTO VA AU ZOO") {
        return false;
    }
    if (operation != "CB") {
        return false;
    }
    if (description != "CB TOTO VA AU ZOO") {
        return false;
    }
    return true;
}

bool test_parseDescription_1() {
    std::string entity;
    std::string operation;
    std::string description;
    if (!parseDescription("CB TOTO VA AU ZOO 25 03 12", entity, operation, description)) {
        return false;
    }
    if (entity != "TOTO VA AU ZOO") {
        return false;
    }
    if (operation != "CB") {
        return false;
    }
    if (description != "CB TOTO VA AU ZOO") {
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v)            \
    if (vTest == std::string(#v)) \
    return v()

#define IfTestCollectionExist(v, str)        \
    if (vTest.find(#v) != std::string::npos) \
    return v(str)

bool Test_Module(const std::string& vTest) {
    IfTestExist(test_parseDescription_0);
    else IfTestExist(test_parseDescription_1);
    return true;
}