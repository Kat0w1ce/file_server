#include <iostream>

#include "client.h"
int main(int argc, char const *argv[]) {
    if (argc < 5) {
        std::cout << "too few parameters" << std::endl;
        return 0;
    }
    std::cout << std::stoi(argv[2]) << '\n';
    client c(argv[1], std::stoi(argv[2]), argv[3], argv[4]);
    c.run();
    return 0;
}
