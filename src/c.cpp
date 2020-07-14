#include "client.h"
int main(int argc, char const *argv[])
{
    client c;
    c.run(std::string(argv[1]));
    return 0;
}
