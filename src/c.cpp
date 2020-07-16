#include "client.h"
int main(int argc, char const *argv[]) {
    client c;
    auto tmp = c.build_cmd(argc, argv);
    c.send_cmd(tmp.second);

    if (tmp.first == 1) c.get(argv[3]);
    if (tmp.first == 2) c.send(argv[2]);
    if (tmp.first == 3) c.get_dir(argv[3]);
    if (tmp.first == 4) c.send_dir(argv[2]);
    return 0;
}
