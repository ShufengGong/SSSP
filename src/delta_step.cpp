//
// Created by gongsf on 2019/11/5.
//

#include <iostream>

#include "sssp_option.hpp"
#include "delta_step_single.hpp"
#include "delta_step_par.hpp"
#include "delta_step_dis.hpp"

using namespace std;
using namespace sssp;

int main(int argc, char *argv[]) {

    sssp_option option("delta_setp");

    if (option.parse_command(argc, argv) == false) {
        cout << "parse command failure" << endl;
        return 0;
    }
    if(option.model == 0){
        delta_step_single delta_step_single_(&option);
        delta_step_single_.run();
    }

    if(option.model == 1){
        delta_step_par delta_step_multhre_(&option);
        delta_step_multhre_.run();
    }

    if(option.model == 2){
        cout << "run distributed delta step" << endl;
        delta_step_dis delta_step_dis_(&option);
        delta_step_dis_.run(&argc, &argv);
    }

    return 0;
}
