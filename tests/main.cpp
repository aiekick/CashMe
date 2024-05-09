#include <string>
#include <src/Test_CashMe.h>

int main(int argc, char** argv) {
    if (argc > 1) {
        printf("Exec test : %s\n", argv[1]);
        return Test_CashMe(argv[1]) ? 0 : 1;
    }
    return Test_CashMe("Test_CashMe_0") ? 0 : 1;
}