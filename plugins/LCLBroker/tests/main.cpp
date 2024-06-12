#include <string>
#include <src/Test_Module.h>

int main(int argc, char** argv) {
    if (argc > 1) {
        printf("Exec test : %s\n", argv[1]);
        return Test_Module(argv[1]) ? 0 : 1;
    }
    return Test_Module("test_parseDescription_0") ? 0 : 1;
}