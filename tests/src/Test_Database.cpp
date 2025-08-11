#include <src/Test_Database.h>
#include <Models/DataBase.h>
#include <ezlibs/ezCTest.hpp>

class DataBaseTester {
public:
    
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*bool Test_Database_buildInsertIfNotExistQuery() {
    const auto& query = DataBaseTester::buildInsertIfNotExistQuery("banks", {{"number", "30002"}, {"name", "LCL"}});
    CTEST_ASSERT(query == "INSERT INTO banks (number, name) SELECT '30002', 'LCL' WHERE NOT EXISTS (SELECT 1 FROM banks WHERE number = '30002' and name = 'LCL');");
    return true;
}*/

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v)            \
    if (vTest == std::string(#v)) \
    return v()

bool Test_Database(const std::string& vTest) {
    //IfTestExist(Test_Database_buildInsertIfNotExistQuery);
    return true;
}
