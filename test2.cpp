#include <iostream>

int main() {
    for (int i = 1; i <= 1000000; ++i) {
        if (i % 1000 == 0) {
            std::cout << i << std::endl;
        }
    }

    return 0;
}