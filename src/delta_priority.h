//
// Created by gongsf on 18/3/19.
//

#ifndef SSSP_DELTA_PRIORITY_H
#define SSSP_DELTA_PRIORITY_H

#include <math.h>
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include "weight_graph.h"

using namespace std;


void iterate_delta_priority(weight_graph *graph, int source);

void iterate_delta_priority2(weight_graph *graph, int source);

void delta_priority(int argc, char *argv[]) {

    string input;
    string output;
    int source = -1;
    int vertex_num = -1;

    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--in") {
            input = argv[++i];
        }

        if (arg == "--out") {
            output = argv[++i];
        }

        if (arg == "--source") {
            source = atoi(argv[++i]);
        }

        if (arg == "--vertex_num") {
            vertex_num = atoi(argv[++i]);
        }
    }

    weight_graph *graph = new weight_graph(vertex_num);
    graph->init_vertex(input);

    clock_t start = clock();
    iterate_delta_priority(graph, source);
    clock_t end = clock();
    float time = (float) (end - start) / CLOCKS_PER_SEC;
    cout << "iteration finished, used time " << time << endl;
    graph->dump_result(output);
}

void iterate_delta_priority2(weight_graph *graph, int source) {
    int vertex_num = graph->vertex_num;
    bool is_end = false;

    vector<float> delta = vector<float>(vertex_num, numeric_limits<float>::max());
    vector<float> dis = vector<float>(vertex_num, numeric_limits<float>::max());
    vector<int> v_iter = vector<int>(vertex_num, 0);
    vector<bool> in_cand = vector<bool>(vertex_num, false);

    queue<int> active_v;
    vector<int> candidate_v;
    vector<int> candidate_v2;

    delta[source] = 0;
    candidate_v.push_back(source);

    clock_t start = clock();

    for (int k = 0; !is_end; k++) {

        cout << "iterate" << k << endl;
        is_end = true;
        int size = candidate_v.size();
//        cout << size << endl;
        for (int i = 0; i < size; i++) {
            //send to neighbor
            int index_ = candidate_v[i];
            dis[index_] = delta[index_];
            in_cand[index_] = false;
            v_iter[index_]++;
            delta[index_] = numeric_limits<float>::max();
            vector<pair<int, float>> neighbors = graph->g[index_];
            for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                pair<int, float> neighbor = *it;
                int id = neighbor.first;
                //update the delta of neighbor
                delta[id] = min(delta[id], neighbor.second + dis[index_]);
                if(v_iter[id] >= 1 && !in_cand[id]){
                    active_v.push(id);
                    in_cand[id] = true;
                }else if(!in_cand[id]){
                    candidate_v2.push_back(id);
                    in_cand[id] = true;
                }
            }
            is_end = false;
        }

        while(!active_v.empty()){
            int index_ = active_v.front();
            active_v.pop();
            in_cand[index_] = false;
            if (delta[index_] < dis[index_]) {
                dis[index_] = delta[index_];
                v_iter[index_]++;
                delta[index_] = numeric_limits<float>::max();
                vector<pair<int, float>> neighbors = graph->g[index_];
                for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                    pair<int, float> neighbor = *it;
                    int id = neighbor.first;
                    //update the delta of neighbor
                    delta[id] = min(delta[id], neighbor.second + dis[index_]);
                    if(v_iter[id] >= 1 && !in_cand[id]){
                        active_v.push(id);
                        in_cand[id] = true;
                    }else if(!in_cand[id]){
                        candidate_v2.push_back(id);
                        in_cand[id] = true;
                    }
                }
                is_end = false;
            }
        }



        candidate_v = candidate_v2;
//        cout << time << endl;
        candidate_v2.clear();

//        if (is_end) {
//            for (int i = 0; i < vertex_num; i++) {
//                if (delta[i] < dis[i]) {
//                    dis[i] = delta[i];
//                    v_iter[i]++;
//                    delta[i] = numeric_limits<float>::max();
//                    //send to neighbor
//                    vector<pair<int, float>> neighbors = graph->g[i];
//                    for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
//                        pair<int, float> neighbor = *it;
//                        int id = neighbor.first;
//                        //update the delta of neighbor
//                        delta[id] = min(delta[id], neighbor.second + dis[i]);
//                    }
//                    is_end = false;
//                }
//            }
//        }
    }

//    clock_t end = clock();
//    float time = (float) (end - start) / CLOCKS_PER_SEC;

    graph->set_dis(dis);
    graph->set_v_iter(v_iter);

}

//simulate maiter, i.e., multi-thread 
void iterate_delta_priority(weight_graph *graph, int source) {
    int vertex_num = graph->vertex_num;
    bool is_end = false;

    vector<float> delta = vector<float>(vertex_num, numeric_limits<float>::max());
    vector<float> dis = vector<float>(vertex_num, numeric_limits<float>::max());
    vector<int> v_iter = vector<int>(vertex_num, 0);
    vector<bool> in_cand = vector<bool>(vertex_num, false);

    queue<int> active_v;
    vector<int> candidate_v;
    vector<int> candidate_v2;

    delta[source] = 0;

    for (int k = 0; !is_end; k++) {

//        cout << "iterate" << k << endl;
        is_end = true;

//        queue<int> candidate;

        for (int i = 0; i < vertex_num; i++) {
            if (delta[i] < dis[i] && !in_cand[i]) {
                dis[i] = delta[i];
                v_iter[i]++;
//                delta[i] = numeric_limits<float>::max();
                //send to neighbor
                vector<pair<int, float>> neighbors = graph->g[i];
                for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                    pair<int, float> neighbor = *it;
                    int id = neighbor.first;
                    //update the delta of neighbor
                    delta[id] = fmin(delta[id], neighbor.second + dis[i]);
                    if(delta[id] < dis[id]){
                        if(v_iter[id] >= 1 && !in_cand[id]){
                            active_v.push(id);
                            in_cand[id] = true;
                        }
                    }
                }
                is_end = false;
            }

        }
        while (!active_v.empty()) {
            int index_ = active_v.front();
            active_v.pop();
            in_cand[index_] = false;
            if (delta[index_] < dis[index_]) {
                dis[index_] = delta[index_];
                v_iter[index_]++;
//                delta[index_] = numeric_limits<float>::max();
                vector<pair<int, float>> neighbors = graph->g[index_];
                for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                    pair<int, float> neighbor = *it;
                    int id = neighbor.first;
                    //update the delta of neighbor
                    delta[id] = min(delta[id], neighbor.second + dis[index_]);
                    if(delta[id] < dis[id]) {
                        if (v_iter[id] >= 1 && !in_cand[id]) {
                            active_v.push(id);
                            in_cand[id] = true;
                        }
                    }
                }
            }else{
                cout << "delta " << index_ << " " << delta[index_] << " dis " << dis[index_] << endl;
                exit(0);
            }
        }
    }

    graph->set_dis(dis);
    graph->set_v_iter(v_iter);

}

vector<int> get_priority_v(vector<float> &dis, vector<float> &delta, vector<int> &v_iter, int vertex_num) {
    vector<int> candidate1;
    vector<int> candidate2;
    for (int i = 0; i < vertex_num; i++) {
        if (delta[i] < dis[i]) {
            candidate1.push_back(i);
            if (v_iter[i] > 1) {
                candidate2.push_back(i);
            }
        }
    }
    return candidate1;
//    if (candidate2.size() > 50) {
//        return candidate2;
//    } else {
//        return candidate1;
//    }
}

#endif //SSSP_DELTA_PRIORITY_H
