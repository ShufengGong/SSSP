//
// Created by gongsf on 2/10/19.
//

#ifndef DCDDPP_AB_SSSP_MULTHRE_HPP
#define DCDDPP_AB_SSSP_MULTHRE_HPP

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


struct ab_sssp_multhre {

private:

    atomic_bool is_end;
    atomic_int finish_num;
    atomic_bool is_continue;

    vector<tbb::atomic<float>> delta;
    vector<float> dis;
    vector<int> v_iter;
    vector<vector<pair<int, float> > > graph;

    vector<tbb::atomic<bool>> in_cand;
    vector<vector<int>> candidate;
    vector<vector<int>> active_cand;

    vector<bool> is_continues;

public:

    sssp_option *option;

    ab_sssp_multhre(sssp_option *option_) {
        option = option_;

        if (option->num_vertex == -1 || option->num_thread == -1 || option->source == -1) {
            cout << "invalid paramete!" << endl;
            exit(0);
        }

        int vertex_num = option->num_vertex;

        is_end = false;
        finish_num = 0;
        is_continue = true;

        in_cand = vector<tbb::atomic<bool>>(vertex_num, false);
        candidate = vector<vector<int>>(option->num_thread);
        active_cand = vector<vector<int>>(option->num_thread);

        is_continues = vector<bool>(option->num_thread, false);

        delta = vector<tbb::atomic<float>>(vertex_num, numeric_limits<float>::max());
        dis = vector<float>(vertex_num, numeric_limits<float>::max());
        v_iter = vector<int>(vertex_num, 0);
        graph = vector<vector<pair<int, float>>>(vertex_num);
    }

    ~ab_sssp_multhre() {
        delta.clear();
        dis.clear();
        v_iter.clear();
        graph.clear();
        vector<tbb::atomic<float>>(delta).swap(delta);
        vector<float>(dis).swap(dis);
        vector<int>(v_iter).swap(v_iter);
        vector<vector<pair<int, float>>>(graph).swap(graph);
    }

    void check_continue() {
        bool flag = true;
        for (int i = 0; i < option->num_thread; i++) {
            flag = flag && is_continues[i];
        }
        if (flag) {
            is_continue = false;
        }
    }

    void check_term() {
        bool flag = true;
        for (int i = 0; i < option->num_vertex; i++) {
            if (delta[i] < dis[i]) {
                flag = false;
            }
        }
        if (flag) {
            is_end = true;
        }
    }

    int run() {

        boost::barrier barrier(option->num_thread);

        cout << "start ab sssp algorithm!" << endl;
        int vertex_num = option->num_vertex;

        delta[option->source] = 0;
        active_cand[option->source % option->num_thread].push_back(option->source);

        std::function<void(int)> updater_func = [&](int thread_index) -> void {

            int num_thread = option->num_thread;
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
            }else{
                cout << "can not open the input file" << endl;
            }
            cout << thread_index << "init finished" << endl;

            barrier.wait();

            ofstream log(option->output + to_string(thread_index));

            string s = to_string(thread_index) + " start update ";
            cout << s << endl;
            clock_t start = clock();

            vector<int> candidate2;
            vector<int> active_cand2;
            for (int k = 0; !is_end; k++) {
                int active_size = active_cand[thread_index].size();
                for (int i = 0; i < active_size; i++) {
                    int c_id = active_cand[thread_index][i];
                    in_cand[c_id] = false;
                    if (delta[c_id] < dis[c_id]) {
                        dis[c_id] = delta[c_id];
                        v_iter[c_id]++;
                        //send to neighbor
                        vector<pair<int, float>> neighbors = graph[c_id];
                        for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                            pair<int, float> neighbor = *it;
                            int id = neighbor.first;
                            //update the delta of neighbor
                            atomic_update(delta[id], neighbor.second + dis[c_id]);
                            if (delta[id] < dis[id] && !in_cand[id]) {
                                if (v_iter[id] >= 1) {
                                    candidate[id % num_thread].push_back(id);
                                    in_cand[id] = true;
                                } else {
                                    active_cand2.push_back(id);
                                    in_cand[id] = true;
                                }
                            }
                        }
                    }
                }

                if (!is_continue) {
                    is_continue = true;
                }
                is_continues[thread_index] = false;
                barrier.wait();

                while (is_continue) {
                    int cand_size = candidate[thread_index].size();
                    for (int i = 0; i < cand_size; i++) {
                        int c_id = candidate[thread_index][i];
                        in_cand[c_id] = false;
                        if (delta[c_id] < dis[c_id]) {
                            dis[c_id] = delta[c_id];
                            v_iter[c_id]++;
                            vector<pair<int, float>> neighbors = graph[c_id];
                            for (vector<pair<int, float>>::iterator it = neighbors.begin();
                                 it != neighbors.end(); ++it) {
                                pair<int, float> neighbor = *it;
                                int id = neighbor.first;
                                //update the delta of neighbor
                                atomic_update(delta[id], neighbor.second + dis[c_id]);
                                if (delta[id] < dis[id] && !in_cand[id]) {
                                    if (v_iter[id] >= 1) {
                                        candidate2.push_back(id);
                                        in_cand[id] = true;
                                    } else {
                                        active_cand2.push_back(id);
                                        in_cand[id] = true;
                                    }
                                }
                            }
                        }
                    }
                    candidate[thread_index] = candidate2;
                    candidate2.clear();
                    if (candidate[thread_index].size() == 0) {
                        is_continues[thread_index] = true;
                        check_continue();
                    }
                }

                active_cand[thread_index] = active_cand2;
                active_cand2.clear();
                barrier.wait();
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

        while (!is_end) {
            this_thread::yield();
            check_term();
        }

        for (int i = 0; i < option->num_thread; i++) {
            update_thread[i].join();
        }

        while (finish_num < option->num_thread) {
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

#endif //DCDDPP_AB_SSSP_MULTHRE_HPP
