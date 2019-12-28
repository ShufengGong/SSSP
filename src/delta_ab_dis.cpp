//
// Created by gongsf on 12/4/19.
//

#include "delta_ab_dis.hpp"
#include "sssp_option.hpp"
#include "delta_ab_dis_im.hpp"

int main(int argc, char *argv[]){
    sssp_option option("delta_setp");

    if (option.parse_command(argc, argv) == false) {
        cout << "parse command failure" << endl;
        return 0;
    }

    if(option.model == 1){
        cout << "delta step blocking" << endl;
        delta_ab_dis delta_ab_dis_(&option);
        delta_ab_dis_.run(&argc, &argv);
    } else if(option.model == 2){
        cout << "improved delta step blocking" << endl;
        delta_ab_dis_im delta_ab_dis_im_(&option);
        delta_ab_dis_im_.run(&argc, &argv);
    }


}

