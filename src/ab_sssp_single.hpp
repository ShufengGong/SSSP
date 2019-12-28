//
// Created by gongsf on 21/10/19.
//

#ifndef SSSP_AB_SSSP_SINGLE_HPP
#define SSSP_AB_SSSP_SINGLE_HPP

#include "weight_graph.h"
#include "util.h"
#include <cmath>
#include "sssp_option.hpp"

struct ab_sssp_single{
public:

    sssp_option *option;
    ab_sssp_single(sssp_option *option_){
        option = option_;
        if(option->source == -1 || option->source > option->num_vertex){
            cout << "source " << option->source << endl;
            cout << "vertex num" << option->num_vertex << endl;
            cout << "invalid parameter source vertex id" << endl;
            exit(0);
        }

        if(option->input.empty() || option->output.empty()){
            cout << "the input or output path is empty" << endl;
            exit(0);
        }
    }

    ~ab_sssp_single(){

    }

    void run(){
        int vertex_num = option->num_vertex;
        bool is_end = false;

        vector<tbb::atomic<float>> delta = vector<tbb::atomic<float>>(vertex_num, numeric_limits<float>::max());
        vector<float> dis = vector<float>(vertex_num, numeric_limits<float>::max());
        vector<int> v_iter = vector<int>(vertex_num, 0);
        vector<bool> in_cand = vector<bool>(vertex_num, false);
        weight_graph *graph = new weight_graph(vertex_num);
        graph->init_vertex(option->input);
        cout << "init finished" << endl;

        delta[option->source] = 0;

        clock_t start = clock();

        vector<int> active_cand;
        vector<int> active_cand2;
        active_cand.push_back(option->source);
        for (int k = 0; !is_end; k++) {

//            queue<int> candidate;
            vector<int> candidate;
            vector<int> candidate2;
            int act_size = active_cand.size();
            for (int i = 0; i < act_size; i++) {
                int c_id = active_cand[i];
                in_cand[c_id] = false;
                if (delta[c_id] < dis[c_id]) {
                    if(dis[c_id] != numeric_limits<float>::max()){
                        cout << c_id << " " << dis[c_id] << endl;
                    }
                    dis[c_id] = delta[c_id];
                    v_iter[c_id]++;
                    //send to neighbor
                    vector<pair<int, float>> neighbors = graph->g[c_id];
                    for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                        pair<int, float> neighbor = *it;
                        int id = neighbor.first;
                        //update the delta of neighbor
                        delta[id] = fmin(delta[id], neighbor.second + dis[c_id]);
                        if (delta[id] < dis[id] && !in_cand[id]) {
                            if(v_iter[id] >= 1){
                                candidate.push_back(id);
                                in_cand[id] = true;
                            }else{
                                active_cand2.push_back(id);
                                in_cand[id] = true;
                            }
                        }
                    }
                }
            }
            while (candidate.size() != 0) {
                int size = candidate.size();
                for(int i = 0; i < size; i++){
                    int c_id = candidate[i];
                    in_cand[c_id] = false;
                    dis[c_id] = delta[c_id];
                    v_iter[c_id]++;
                    vector<pair<int, float>> neighbors = graph->g[c_id];
                    for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                        pair<int, float> neighbor = *it;
                        int id = neighbor.first;
                        //update the delta of neighbor
                        delta[id] = fmin(delta[id], neighbor.second + dis[c_id]);
                        if (delta[id] < dis[id] && !in_cand[id]) {
                            if(v_iter[id] >= 1){
                                candidate2.push_back(id);
                                in_cand[id] = true;
                            }else{
                                active_cand2.push_back(id);
                                in_cand[id] = true;
                            }
                        }
                    }
                }
                candidate = candidate2;
                candidate2.clear();
            }
            candidate.clear();
            active_cand = active_cand2;
            active_cand2.clear();
            if(active_cand.size() == 0){
                is_end = true;
            }else{
                is_end = false;
            }
        }

        clock_t end = clock();
        float time = (float) (end - start) / CLOCKS_PER_SEC;
        cout << "iteration finished in " << time << "s" << endl;
        graph->set_dis(dis);
        graph->set_v_iter(v_iter);

        graph->dump_result(option->output);
    }
};

#endif //SSSP_AB_SSSP_SINGLE_HPP
