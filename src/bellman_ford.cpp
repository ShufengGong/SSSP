//
// Created by gongsf on 12/18/19.
//


#include "sssp_option.hpp"
#include "bellman_ford.hpp"

using namespace std;
using namespace sssp;

int main(int argc, char *argv[])
{
    sssp_option option("bellman_ford");

    if (option.parse_command(argc, argv) == false) {
        cout << "parse command failure" << endl;
        return 0;
    }

    bellman_ford bel(&option);
    bel.run(&argc, &argv);

    return 0;
}