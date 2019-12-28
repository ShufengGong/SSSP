//
// Created by gongsf on 10/4/19.
//

#ifndef SSSP_DIJKSTRA_H
#define SSSP_DIJKSTRA_H

#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include "weight_graph.h"
#include <time.h>
#include <queue>
#include "MutablePriorityQueue.h"
#include "sssp_option.hpp"

using namespace sssp;
using namespace std;

void iterate_dijkstra(weight_graph *graph, int source);

void dijkstra(int argc, char *argv[]){

    int source = -1;
    int vertex_num = -1;

    cout << "dijkstra start" << endl;

    sssp_option option("dijkstra option");
    option.parse_command(argc, argv);

    source = option.source;
    vertex_num = option.num_vertex;

    if(source == -1 || vertex_num == -1){
        cout << "source" << source << endl;
        cout << "vertex num" << vertex_num << endl;
        cout << "invalid parameter" << endl;
        return;
    }

    weight_graph *graph = new weight_graph(vertex_num);
    graph->init_vertex(option.input);

    clock_t start = clock();
    iterate_dijkstra(graph, source);
    clock_t end = clock();
    float time = (float) (end - start) / CLOCKS_PER_SEC;
    cout << "iteration finished, used time " << time << endl;
    graph->dump_result(option.output);
}

struct less_than_key{
    inline bool operator()(pair<int, float> *node1, pair<int, float> *node2){
        if(node1->second > node2->second)
            return 1;
        else
            return 0;
    }
};

void iterate_dijkstra(weight_graph *graph, int source){
    int vertex_num = graph->vertex_num;

    vector<float> dis = vector<float>(vertex_num, numeric_limits<float>::max());
    vector<int> v_iter = vector<int>(vertex_num, 0);
    MutablePriorityQueue<int, float> cand_queue = MutablePriorityQueue<int, float>(vertex_num); ;
    vector<bool> in_cand = vector<bool>(vertex_num, false);
    vector<bool> in_result = vector<bool>(vertex_num, false);

    cand_queue.Insert(source, 0);
    in_cand[source] = true;
    for(int i = 0; cand_queue.GetQueueSize() > 0; i++){
//        if(i %10000 == 0){
//            cout << "number " << i << " vertex; " << "cand num " << cand_queue.GetQueueSize() << endl;
//        }
        int index = cand_queue.Peek().key;
        dis[index] = cand_queue.Peek().pri;
        cand_queue.Pop();
        in_cand[index] = false;
        in_result[index] = true;
        vector<pair<int, float>> neighbors = graph->g[index];
        for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
            pair<int, float> neighbor = *it;
            int id = neighbor.first;
            if(!in_result[id]){
                float distance = neighbor.second;
                if(!in_cand[id]){
                    cand_queue.Insert(id, dis[index] + distance);
                    in_cand[id] = true;
                }else{
                    cand_queue.ChangePriority(id, min(cand_queue.GetPriority(id), dis[index]+ distance));
                }
                v_iter[id]++;
            }
        }
    }
    graph->set_dis(dis);
    graph->set_v_iter(v_iter);
}

#endif //SSSP_DIJKSTRA_H
