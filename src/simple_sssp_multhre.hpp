//
// Created by gongsf on 2/10/19.
//

#ifndef SIMPLE_SSSP_MULTHRE_HPP
#define SIMPLE_SSSP_MULTHRE_HPP

#include <thread>
#include <queue>
#include <atomic>
#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/thread/barrier.hpp>

#include <tbb/tbb.h>

#include "sssp_option.hpp"
#include "util.h"

using namespace std;
using namespace sssp;
using tbb::atomic;


struct simple_sssp_multhre {

private:

    atomic_bool is_end;
    atomic_int finish_num;

    vector<tbb::atomic<float>> delta;
    vector<float> dis;
    vector<int> v_iter;
    vector<vector<pair<int, float> > > graph;

public:

    sssp_option *option;

    simple_sssp_multhre(sssp_option *option_) {
        option = option_;

        if (option->num_vertex == -1 || option->num_thread == -1 || option->source == -1) {
            cout << "invalid paramete!" << endl;
            exit(0);
        }

        int vertex_num = option->num_vertex;

        is_end = false;
        finish_num = 0;

        delta = vector<tbb::atomic<float>>(vertex_num, numeric_limits<float>::max());
        dis = vector<float>(vertex_num, numeric_limits<float>::max());
        v_iter = vector<int>(vertex_num, 0);
        graph = vector<vector<pair<int, float>>>(vertex_num);
    }

    ~simple_sssp_multhre() {
        delta.clear();
        dis.clear();
        v_iter.clear();
        graph.clear();
        vector<tbb::atomic<float>>(delta).swap(delta);
        vector<float>(dis).swap(dis);
        vector<int>(v_iter).swap(v_iter);
        vector<vector<pair<int, float>>>(graph).swap(graph);
    }

    void check_term() {
        bool flag = true;
        for(int i = 0; i < option->num_vertex; i++){
            if(delta[i] < dis[i]){
                flag = false;
            }
        }
        if(flag){
            is_end = true;
//            cout << "finish" << endl;
        }else{
//            cout << "not finish" << endl;
        }
    }

    int run() {

        boost::barrier barrier(option->num_thread);

        cout << "start ab sssp algorithm!" << endl;
        int vertex_num = option->num_vertex;

        delta[option->source] = 0;

        std::function<void(int)> updater_func = [&](int thread_index) -> void {

            ifstream infile((option->input + "/part" + to_string(thread_index)).c_str());
            if (infile.is_open()) {
                string str;
                while (getline(infile, str)) {
                    boost::trim(str);
                    int pos = str.find_first_of("\t");
                    int src = atoi(str.substr(0, pos).c_str());

                    string remain = str.substr(pos + 1);
                    vector<pair<int, float>> neighbor;
                    if (*remain.end() != ' ') {
                        remain += " ";
                    }
                    while ((pos = remain.find_first_of(" ")) != remain.npos) {
                        int target;
                        float weight;
                        if (pos > 0) {
                            string cur = remain.substr(0, pos);
                            int cut = cur.find_first_of(",");
                            target = atoi(cur.substr(0, cut).c_str());
                            weight = atof(cur.substr(cut + 1).c_str());
                            neighbor.push_back(make_pair(target, weight));
                        }
                        remain = remain.substr(pos + 1);
                    }
                    graph[src] = neighbor;
                }
            }
            cout << thread_index << "init finished" << endl;

            barrier.wait();

            ofstream log(option->output + to_string(thread_index));

            string s = to_string(thread_index) + " start update ";
            cout << s << endl;
            clock_t start = clock();
            int step = option->num_thread;

            for (int k = 0; !is_end; k++) {
                log << thread_index << " thread iteration " << k << "times" << endl;
                //expend active sub graph
                for (int i = thread_index; i < vertex_num; i += step) {
                    if (delta[i] < dis[i]) {
                        dis[i] = delta[i];
                        v_iter[i]++;
                        //send to neighbor
                        vector<pair<int, float>> neighbors = graph[i];
                        for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                            pair<int, float> neighbor = *it;
                            int id = neighbor.first;
                            //update the delta of neighbor
                            atomic_update(delta[id], neighbor.second + dis[i]);
                        }
                    }
                }
//                barrier.wait();
//                check_term();
            }

            log.close();
            clock_t end = clock();
            float time = (float) (end - start) / CLOCKS_PER_SEC;
            cout << "iteration finished in " << time << "s" << endl;
            finish_num++;

        };// end of definining update function

        std::thread update_thread[option->num_thread];
        for (int i = 0; i < option->num_thread; i++) {
            update_thread[i] = std::thread(updater_func, i);
        }
        cout << "create thread finished" << endl;

        while(!is_end){
            this_thread::yield();
            check_term();
        }

        for (int i = 0; i < option->num_thread; i++) {
            update_thread[i].join();
        }

        while(finish_num < option->num_thread){
            this_thread::yield();
        }

        ofstream outfile(option->output + "result");
        for (int i = 0; i < vertex_num; i++) {
            outfile << i << "\t" << v_iter[i] << "\t";
            outfile << dis[i] << endl;
        }
        outfile.close();

        return 0;

    }
};

#endif //SIMPLE_SSSP_MULTHRE_HPP
