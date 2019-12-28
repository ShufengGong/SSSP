//
// Created by gongsf on 23/10/19.
//

#include "simple_sssp_multhre.hpp"

using namespace std;
using namespace sssp;

int main(int argc, char** argv){
    sssp_option option("simple_sssp");

    if (option.parse_command(argc, argv) == false) {
        cout << "parse command failure" << endl;
        return 0;
    }

    simple_sssp_multhre simple_sssp_multhre_(&option);
    simple_sssp_multhre_.run();
}

