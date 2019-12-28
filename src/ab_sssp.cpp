//
// Created by gongsf on 26/9/19.
//

#include <iostream>
//#include <vector>
#include "sssp_option.hpp"
#include "ab_sssp_multhre.hpp"
#include "ab_sssp_single.hpp"

using namespace std;
using namespace sssp;

int main(int argc, char *argv[]) {

    sssp_option option("ab_sssp");

    if (option.parse_command(argc, argv) == false) {
        cout << "parse command failure" << endl;
        return 0;
    }

    if(option.model == 0){
        ab_sssp_single ab_sssp_single_(&option);
        ab_sssp_single_.run();
    }else if(option.model == 1){
        ab_sssp_multhre ab_sssp_multhre_(&option);
        ab_sssp_multhre_.run();
    }else{
        cout << "invalid model" << endl;
    }

}

