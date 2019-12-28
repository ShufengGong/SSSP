//
// Created by gongsf on 21/3/19.
//

#ifndef SSSP_DELTA_STEP_SINGLE_HPP
#define SSSP_DELTA_STEP_SINGLE_HPP

#include <unordered_set>
#include <vector>

#include "weight_graph.h"
#include "util.h"
#include "sssp_option.hpp"

using namespace sssp;

struct delta_step_single {

private:
    void iterate_delta_step() {

        relax(source, 0);
        int iter = 0;
        for (int i = 0; i < delta_length; i++) {
            iter++;
            cout << "iteration " << iter << endl;
            unordered_set<int> r_;
            while (!bucket[i].empty()) {
                vector<pair<int, float>> req = find_request(bucket[i], true);
                r_.insert(bucket[i].begin(), bucket[i].end());
                bucket[i].clear();
                relax_request(req);
            }
            vector<pair<int, float>> req = find_request(r_, false);
            relax_request(req);
        }
        graph->set_dis(tent);
        graph->set_v_iter(v_iter);
    }

    vector<pair<int, float>> find_request(unordered_set<int> &b_, bool kind) {
        vector<pair<int, float>> req;
        for (auto it_ = b_.begin(); it_ != b_.end(); it_++) {
            int v = *it_;
            vector<pair<int, float>> neighbors = graph->g[v];
            for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
                pair<int, float> neighbor = *it;
                if (kind) {
                    if (neighbor.second < delta) {
                        req.push_back(make_pair(neighbor.first, neighbor.second + tent[v]));
                    }
                } else {
                    if (neighbor.second > delta) {
                        req.push_back(make_pair(neighbor.first, neighbor.second + tent[v]));
                    }
                }
            }
        }
        return req;
    }

    void relax_request(vector<pair<int, float>> &req) {
        for (int i = 0; i < req.size(); i++) {
            relax(req[i].first, req[i].second);
        }
    }

    inline void relax(int index, float weight) {
        if (weight < tent[index]) {
            if (tent[index] != numeric_limits<float>::max()) {
                int b_index = (int) floor(tent[index] / delta);
                bucket[b_index].erase(index);
            }
            int b_index_ = (int) floor(weight / delta);
            bucket[b_index_].insert(index);
            tent[index] = weight;
            v_iter[index]++;
        }
    }

public:
    sssp_option *option;

    int vertex_num;
    float delta;
    int delta_length;
    int source;
    weight_graph *graph;
    vector<float> tent;
    vector<int> v_iter;
    vector<unordered_set<int>> bucket;

    delta_step_single(sssp_option *option_) {
        option = option_;
        if (option->source == -1 || option->source > option->num_vertex) {
            cout << "source " << option->source << endl;
            cout << "vertex num" << option->num_vertex << endl;
            cout << "invalid parameter source vertex id" << endl;
            exit(0);
        }

        if (option->input.empty() || option->output.empty()) {
            cout << "the input or output path is empty" << endl;
            exit(0);
        }

        if (option->num_vertex == -1 || option->delta == -1 || option->delta_length == -1 || option->source == -1) {
            cout << "error parameters" << endl;
            exit(0);
        }

        vertex_num = option->num_vertex;
        delta = option->delta;
        delta_length = option->delta_length;
        source = option->source;
        tent = vector<float>(vertex_num, numeric_limits<float>::max());
        v_iter = vector<int>(vertex_num, 0);
        bucket = vector<unordered_set<int>>(option->delta_length, unordered_set<int>());
    }

    ~delta_step_single() {
        tent.clear();
        v_iter.clear();
        bucket.clear();
        vector<float>(tent).swap(tent);
        vector<int>(v_iter).swap(v_iter);
        vector<unordered_set<int>>(bucket).swap(bucket);
    }

    void run() {

        cout << "delta step start" << endl;

        graph = new weight_graph(option->num_vertex);
        graph->init_vertex(option->input);

        clock_t start = clock();
        bucket[0].insert(source);
        tent[source] = 0;
        v_iter[source]++;
        for (int i = 0; i < delta_length; i++) {
//            if(i % 100 == 0){
//                cout << "iteration " << i << endl;
//            }
            vector<pair<int, float>> r_;
            while (!bucket[i].empty()) {
//                cout << "i" << endl;
                unordered_set<int> b_ = bucket[i];
                bucket[i].clear();
                for (auto it_ = b_.begin(); it_ != b_.end(); it_++) {
                    int v = *it_;
                    vector<pair<int, float>> neighbors = graph->g[v];
                    for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
                        pair<int, float> neighbor = *it;
                        if (neighbor.second < delta) {
                            float weight = neighbor.second + tent[v];
                            if (weight < tent[neighbor.first]) {
                                if (tent[neighbor.first] != numeric_limits<float>::max()) {
                                    int b_index = (int) floor(tent[neighbor.first] / delta);
                                    bucket[b_index].erase(neighbor.first);
                                }
                                int b_index_ = (int) floor(weight / delta);
                                bucket[b_index_].insert(neighbor.first);
                                tent[neighbor.first] = weight;
                                v_iter[neighbor.first]++;
                            }
                        } else {
                            float weight = neighbor.second + tent[v];
                            if (weight < tent[neighbor.first]) {
                                r_.push_back(make_pair(neighbor.first, weight));
                            }
                        }
                    }
                }
            }
            for (auto j = 0; j < r_.size(); j++) {
                int v = r_[j].first;
                float weight = r_[j].second;
                if(weight < tent[v]){
                    if(tent[v] != numeric_limits<float>::max()){
                        int b_index = (int) floor(tent[v]/delta);
                        bucket[b_index].erase(v);
                    }
                    int b_index = (int) floor(weight / delta);
                    bucket[b_index].insert(v);
                    tent[v] = weight;
                    v_iter[v]++;
                }
            }
        }

        graph->set_dis(tent);
        graph->set_v_iter(v_iter);
        clock_t end = clock();
        float time = (float) (end - start) / CLOCKS_PER_SEC;
        cout << "iteration finished, used time " << time << endl;
        graph->dump_result(option->output);
    }
};


#endif //SSSP_DELTA_STEP_SINGLE_HPP
