#include <string>
#include <src/Test_CashMe.h>
#include <ezlibs/ezApp.hpp>

int main(int argc, char** argv) {
    ez::App(argc, argv);
    if (argc > 1) {
        printf("Exec test : %s\n", argv[1]);
        return Test_CashMe(argv[1]) ? 0 : 1;
    }
    return Test_CashMe("Test_Database_buildInsertIfNotExistQuery") ? 0 : 1;
}