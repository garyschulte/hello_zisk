#include <cstdio>

// Global object to test C++ constructors
class Greeter {
public:
    Greeter() {
        printf("C++ Constructor called\n");
    }

    ~Greeter() {
        printf("C++ Destructor called\n");
    }

    void greet() {
        printf("Hello from C++ class!\n");
    }
};

Greeter global_greeter;

int main() {
    printf("Hello, World from bare metal RISC-V!\n");
    printf("This is a C++ application with newlib support\n");

    global_greeter.greet();

    return 0;
}
