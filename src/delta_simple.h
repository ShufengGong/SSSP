//
// Created by gongsf on 15/3/19.
//

#ifndef SSSP_DELTA_SIMPLE_H
#define SSSP_DELTA_SIMPLE_H

#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include "weight_graph.h"
#include <time.h>
#include <queue>

using namespace std;

void iterate_delta_simple(weight_graph *graph, int source);

void delta_simple(int argc, char *argv[]) {

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

        if (arg == "--s") {
            source = atoi(argv[++i]);
        }

        if (arg == "--vnum") {
            vertex_num = atoi(argv[++i]);
        }
    }

    weight_graph *graph = new weight_graph(vertex_num);
    graph->init_vertex(input);

    clock_t start = clock();
    iterate_delta_simple(graph, source);
    clock_t end = clock();
    float time = (float)(end - start) / CLOCKS_PER_SEC;
    cout << "iteration finished, used time " << time << endl;
    graph->dump_result(output);

}

void iterate_delta_simple(weight_graph *graph, int source) {
    int vertex_num = graph->vertex_num;
    bool is_end = false;

    vector<float> delta = vector<float>(vertex_num, numeric_limits<float>::max());
    vector<float> dis = vector<float>(vertex_num, numeric_limits<float>::max());
    delta[source] = 0;

    vector<int> v_iter = vector<int>(vertex_num, 0);

    for (int k = 0; !is_end; k++) {

        cout << "iterate" << k << endl;
        is_end = true;
//        if(k % 2 == 1){
            for (int i = 0; i < vertex_num; i++) {
                // update self
                if (delta[i] < dis[i]) {
                    dis[i] = delta[i];
                    v_iter[i]++;
                    delta[i] = numeric_limits<float>::max();
                    //send to neighbor
                    vector<pair<int, float>> neighbors = graph->g[i];
                    for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                        pair<int, float> neighbor = *it;
                        int id = neighbor.first;
                        //update the delta of neighbor
                        delta[id] = min(delta[id], neighbor.second + dis[i]);
//                    cout << "delta[" << id << "] " << delta[id] << endl;
                    }
                    is_end = false;
                }
            }
//        }else{
//            for (int i = vertex_num - 1; i >= 0; i--) {
//                // update self
//                if (delta[i] < dis[i]) {
//                    dis[i] = delta[i];
//                    delta[i] = numeric_limits<float>::max();
//                    //send to neighbor
//                    vector<pair<int, float>> neighbors = graph->g[i];
//                    for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
//                        pair<int, float> neighbor = *it;
//                        int id = neighbor.first;
//                        //update the delta of neighbor
//                        delta[id] = min(delta[id], neighbor.second + dis[i]);
////                    cout << "delta[" << id << "] " << delta[id] << endl;
//                    }
//                    is_end = false;
//                }
//            }
//        }
    }

    graph->set_dis(dis);
    graph->set_v_iter(v_iter);

}

#endif //SSSP_DELTA_SIMPLE_H
