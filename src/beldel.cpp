//
// Created by gongsf on 12/18/19.
//

#include "beldel.hpp"
using namespace std;

int main(int argc, char *argv[])
{
    sssp_option option("bellman_ford");

    if (option.parse_command(argc, argv) == false) {
        cout << "parse command failure" << endl;
        return 0;
    }

    beldel bd(&option);
    bd.run(&argc, &argv);
    return 0;
}