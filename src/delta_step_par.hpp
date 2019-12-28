//
// Created by gongsf on 2019/11/6.
//

#ifndef SSSP_DELTA_STEP_PAR_HPP
#define SSSP_DELTA_STEP_PAR_HPP

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

struct delta_step_par {

private:

    enum kind {
        light, heavy
    };

    bool is_all_empty(int index) {
        bool is_empty = true;
        for (int i = 0; i < num_thread && is_empty; i++) {
            is_empty = is_empty && buckets[i][index].empty();
        }

        return is_empty;
    }

public:
    sssp_option *option;

    tbb::atomic<int> finish_num;
    int num_vertex;
    int num_thread;
    float delta;
    int delta_length;
    int capacity_req;
    int *size_light_receiver;
    int *size_light_sender;
    int *size_heavy_receiver;
    int *size_heavy_sender;

    vector<float> tent;
    vector<pair<int, float> *> req_light_receiver;
    vector<pair<int, float> *> req_light_sender;
    vector<pair<int, float> *> req_heavy_receiver;
    vector<pair<int, float> *> req_heavy_sender;
    vector<int> v_iter;
    vector<vector<pair<int, float> > > graph;
    vector<vector<unordered_set<int>>> buckets;

    float *time;

    delta_step_par(sssp_option *option_) {
        cout << "start multi-thread delta step" << endl;
        option = option_;
        if (option->num_vertex == -1 || option->num_thread == -1 || option->source == -1) {
            cout << "invalid paramete!" << endl;
            exit(0);
        }

        finish_num = 0;
        num_vertex = option->num_vertex;
        num_thread = option->num_thread;
        delta = option->delta;
        delta_length = option->delta_length;

        tent = vector<float>(num_vertex, numeric_limits<float>::max());
        size_light_receiver = new int[num_thread];
        size_heavy_receiver = new int[num_thread];
        size_light_sender = new int[num_thread];
        size_heavy_sender = new int[num_thread];
        req_light_receiver = vector<pair<int, float> *>(num_thread);
        req_heavy_receiver = vector<pair<int, float> *>(num_thread);
        req_light_sender = vector<pair<int, float> *>(num_thread);
        req_heavy_sender = vector<pair<int, float> *>(num_thread);
        capacity_req = option->req_capacity;
        for (int i = 0; i < num_thread; i++) {
            req_light_receiver[i] = new pair<int, float>[capacity_req];
            size_light_receiver[i] = 0;
            req_heavy_receiver[i] = new pair<int, float>[capacity_req];
            size_heavy_receiver[i] = 0;
            req_light_sender[i] = new pair<int, float>[capacity_req];
            size_light_sender[i] = 0;
            req_heavy_sender[i] = new pair<int, float>[capacity_req];
            size_heavy_sender[i] = 0;
        }
        v_iter = vector<int>(num_vertex, 0);
        graph = vector<vector<pair<int, float>>>(num_vertex);
        buckets = vector<vector<unordered_set<int>>>(num_thread, vector<unordered_set<int>>(option->delta_length,
                                                                                            unordered_set<int>()));
        time = new float[num_thread];
    }

    ~delta_step_par(){
        vector<float>().swap(tent);
        for (int i = 0; i < num_thread; i++) {
            delete []req_light_receiver[i];
            delete []req_heavy_receiver[i];
            delete []req_light_sender[i];
            delete []req_heavy_sender[i];
        }
        vector<pair<int, float> *>().swap(req_light_receiver);
        vector<pair<int, float> *>().swap(req_light_sender);
        vector<pair<int, float> *>().swap(req_heavy_receiver);
        vector<pair<int, float> *>().swap(req_heavy_sender);
        vector<int>().swap(v_iter);
        vector<vector<pair<int, float>>>().swap(graph);
        vector<vector<unordered_set<int>>>().swap(buckets);

        delete []size_light_receiver;
        delete []size_light_sender;
        delete []size_heavy_receiver;
        delete []size_heavy_sender;
    }

    void run() {
        cout << "start delta step multi-thread" << endl;
        boost::barrier barrier(num_thread);
        tent[option->source] = 0;
        v_iter[option->source]++;
        buckets[option->source % num_thread][0].insert(option->source);
        cout << buckets[option->source % num_thread][0].size() << endl;

        std::function<void(int)> update_function = [&](int thread_index) -> void {
            string input_path = option->input + "/part" + to_string(thread_index);
            ifstream infile(input_path.c_str());
            cout << "start init" << endl;
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
                            neighbor.emplace_back(make_pair(target, weight));
                        }
                        remain = remain.substr(pos + 1);
                    }
                    graph[src] = neighbor;
                }
            } else {
                cout << "can not open the graph file" << endl;
            }
            cout << thread_index << " init finished" << endl;

            barrier.wait();

            clock_t start = clock();

            for (int i = 0; i < delta_length; i++) {
                barrier.wait();
                vector<int> r_;
                while (!is_all_empty(i)) {
                    barrier.wait();
                    if (!buckets[thread_index][i].empty()) {
                        for (auto it_ = buckets[thread_index][i].begin();
                             it_ != buckets[thread_index][i].end(); it_++) {
                            int v = *it_;
                            vector<pair<int, float>> neighbors = graph[v];

                            for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
                                pair<int, float> neighbor = *it;
                                if (neighbor.second < delta) {
                                    int k = neighbor.first;
                                    float val = neighbor.second + tent[v];
                                    req_light_receiver[thread_index][size_light_receiver[thread_index]++] = make_pair(k, val);
                                    if(size_light_receiver[thread_index] >= capacity_req){
                                        cout << "light capacity is not enough " << size_light_receiver[thread_index] << endl;
                                    }
                                } else {
                                    int k = neighbor.first;
                                    float val = neighbor.second + tent[v];
                                    req_heavy_receiver[thread_index][size_heavy_receiver[thread_index]++] = make_pair(k, val);
                                    if(size_heavy_receiver[thread_index] >= capacity_req){
                                        cout << "heavy capacity is not enough " << size_heavy_receiver[thread_index] << endl;
                                    }
                                }
                            }
                        }
                        buckets[thread_index][i].clear();
                    }

                    barrier.wait();

                    for(int j = 0; j < num_thread; j++){

                        for(int k = 0; k < size_light_receiver[j]; k++){
                            if(req_light_receiver[j][k].first % num_thread == thread_index){
                                req_light_sender[thread_index][size_light_sender[thread_index]++] = req_light_receiver[j][k];
                            }
                        }
                    }
                    barrier.wait();
                    if (size_light_sender[thread_index] != 0) {
                        for (int j = 0; j < size_light_sender[thread_index]; j++) {
                            pair<int, float> neighbor = req_light_sender[thread_index][j];
                            float weight = neighbor.second;
                            int thread_b = neighbor.first % num_thread;
                            if (weight < tent[neighbor.first]) {
                                if (tent[neighbor.first] != numeric_limits<float>::max()) {
                                    int b_index = (int) floor(tent[neighbor.first] / delta);
                                    buckets[thread_b][b_index].erase(neighbor.first);
                                }
                                int b_index_ = (int) floor(weight / delta);
                                buckets[thread_b][b_index_].insert(neighbor.first);
                                tent[neighbor.first] = weight;
                                v_iter[neighbor.first]++;
                            }
                        }
                    }
                    size_light_sender[thread_index] = 0;
                    size_light_receiver[thread_index] = 0;
                    barrier.wait();
                }
                barrier.wait();
                for(int j = 0; j < num_thread; j++){
                    for(int k = 0; k < size_heavy_receiver[j]; k++){
                        if(req_heavy_receiver[j][k].first % num_thread == thread_index){
                            req_heavy_sender[thread_index][size_heavy_sender[thread_index]++] = req_heavy_receiver[j][k];
                        }
                    }
                }

                barrier.wait();
                if (size_heavy_sender[thread_index] != 0) {
                    for (int j = 0; j < size_heavy_sender[thread_index]; j++) {
                        pair<int, float> req = req_heavy_sender[thread_index][j];
                        float weight = req.second;
                        int v = req.first;
                        int thread_b = v % num_thread;
                        if (weight < tent[v]) {
                            if (tent[v] != numeric_limits<float>::max()) {
                                int b_index = (int) floor(tent[v] / delta);
                                if (b_index >= option->delta_length) {
                                    cout << "index is larger than delta length" << endl;
                                    exit(0);
                                }
                                buckets[thread_b][b_index].erase(v);
                            }

                            int b_index_ = (int) floor(weight / delta);
                            if (b_index_ >= option->delta_length) {
                                cout << "index is larger than delta length" << endl;
                                exit(0);
                            }
                            buckets[thread_b][b_index_].insert(v);
                            tent[v] = weight;
                            v_iter[v]++;
                        }
                    }
                }
                size_heavy_receiver[thread_index] = 0;
                size_heavy_sender[thread_index] = 0;
//                barrier.wait();
            }

            clock_t end = clock();
            time[thread_index] = (float) (end - start) / CLOCKS_PER_SEC;
            finish_num++;
        };

        std::thread update_thread[option->num_thread];
        for (int i = 0; i < option->num_thread; i++) {
            update_thread[i] = std::thread(update_function, i);
        }
        cout << "create thread finished" << endl;

        for (int i = 0; i < option->num_thread; i++) {
            update_thread[i].join();
        }

        while (finish_num < option->num_thread) {
            this_thread::yield();
        }

        for(int i = 0; i < num_thread; i++){
            cout << i << "thread use time " << time[i] << endl;
        }

        ofstream outfile(option->output);
        for (int i = 0; i < num_vertex; i++) {
            outfile << i << "\t" << v_iter[i] << "\t";
            outfile << tent[i] << endl;
        }
        outfile.close();
    }
};

#endif //SSSP_DELTA_STEP_PAR_HPP
