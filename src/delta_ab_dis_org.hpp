//
// Created by gongsf on 12/4/19.
//

#ifndef SSSP_DELTA_AB_DIS_ORG_HPP
#define SSSP_DELTA_AB_DIS_ORG_HPP

#include "sssp_option.hpp"
#include "util.h"
#include "graph_dis.hpp"

#include <mpi.h>
#include <vector>
#include <unordered_set>

using namespace std;
using namespace sssp;

struct delta_ab_dis_org {
private:

    enum kind {
        light, heavy
    };

    bool is_all_empty(int size) {
        int val;
        int result;
        if (size > 0) {
            val = 1;
        } else {
            val = 0;
        }

        MPI_Allreduce(&val, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
//        cout << rank << " - " << result << endl;
        if (result == 0) {
            return true;
        } else {
            return false;
        }
    }

public:

    sssp_option *option;
    int num_task;
    int rank;
    int hostname_len;
    vector<float> tent;
    vector<int> v_iter;
//    vector<bool> stable;

    delta_ab_dis_org(sssp_option *option_) {
        option = option_;

        if (option->num_vertex == -1 || option->source == -1 || option->req_capacity == -1 || option->delta == -1 ||
            option->delta_length == -1 || option->message_length < 2) {
            cout << "invalid paramete!" << endl;
            exit(0);
        }

    }

    int run(int *argc, char ***argv) {
        cout << "init MPI" << endl;
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_task);
        int num_vertex = option->num_vertex;
        float delta = option->delta;
        int delta_length = option->delta_length;
        int capacity_req = option->req_capacity;
        int *size_receiver = new int[num_task];
        int *size_heavy_receiver = new int[num_task];
        vector<pair<int, float> *> req_receiver = vector<pair<int, float> *>(num_task);
        vector<pair<int, float> *> req_heavy_receiver = vector<pair<int, float> *>(num_task);
        pair<int, float> *req_sender = new pair<int, float>[capacity_req];
        pair<int, float> *req_heavy_sender = new pair<int, float>[capacity_req];
        for (int i = 0; i < num_task; i++) {
            size_receiver[i] = 0;
            req_receiver[i] = new pair<int, float>[capacity_req];
            size_heavy_receiver[i] = 0;
            req_heavy_sender = new pair<int, float>[capacity_req];
        }
        graph_dis *graph = new graph_dis(num_vertex / num_task);
        cout << rank << " loading graph" << endl;
        graph->init(option->input + "/part" + to_string(rank), rank, num_task);
        cout << rank << " load garph finished" << endl;
        tent = vector<float>(num_vertex, numeric_limits<float>::max());
        v_iter = vector<int>(num_vertex, 0);
        vector<unordered_set<int>> bucket = vector<unordered_set<int>>(option->delta_length, unordered_set<int>());

        MPI_Barrier(MPI_COMM_WORLD);

        int source = option->source;
        if (source % num_task == rank) {
            tent[source] = 0;
            bucket[0].insert(source);
        }

        ofstream log("logs/proc" + to_string(rank));

        clock_t start = clock();
        for (int i = 0; i < delta_length; i++) {
//            log << rank << " iter " << i << endl;
//            cout << rank << " iter " << i << endl;
            float threshold = delta * (i + 1);
            MPI_Barrier(MPI_COMM_WORLD);
            while (!is_all_empty(bucket[i].size())) {
//                log << rank << " bucket size " << bucket[i].size() << endl;
//                log.flush();
//                MPI_Barrier(MPI_COMM_WORLD);
                for (int j = 0; j < num_task; j++) {
                    size_receiver[j] = 0;
                }
                if (!bucket[i].empty()) {
                    for (auto it_ = bucket[i].begin();
                         it_ != bucket[i].end(); it_++) {
                        int v = *it_;
                        float val = tent[v];
                        int v_host = v % num_task;
                        req_receiver[v_host][size_receiver[v_host]++] = make_pair(v, val);
                        if (size_receiver[v_host] >= capacity_req) {
                            cout << "light capacity is not enough " << size_receiver[v_host] << endl;
                            exit(0);
                        }
                    }
                    bucket[i].clear();
                }
//                log << rank << " send req" << endl;
//                log.flush();
                MPI_Barrier(MPI_COMM_WORLD);
                //send and receive light req
//                log << rank << " receivedddd req " << rec_size << endl;
//                log.flush();
//                MPI_Barrier(MPI_COMM_WORLD);
                int rec_size = gather_data(req_receiver, size_receiver, req_sender, capacity_req, num_task, rank,
                                           option->message_length);
                for (int j = 0; j < rec_size; j++) {
                    pair<int, float> neighbor = req_sender[j];
                    int index = neighbor.first;
                    
                    float weight = neighbor.second;
                    if (index % num_task != rank) {
                        cout << "there is not such vertex id : " << neighbor.first << " in " << "worker " << rank
                             << endl;
                        exit(0);
                    }
                    if (weight < tent[index]) {
                        if (tent[index] != numeric_limits<float>::max()) {
                            int b_index_old = (int) floor(tent[index] / delta);
                            bucket[b_index_old].erase(neighbor.first);
                        }
//                            cout << rank << " : vertex id " << neighbor.first << " : tent index " << index
//                                 << " : bucket index " << b_index_new << endl;
                        tent[index] = weight;
                    }
                    int v = index / num_task;
                    vector<pair<int, float>> &neighbors = (graph->vertices[v]->neighbor);
                    for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
                        pair<int, float> neighbor = *it;
                        int k = neighbor.first;
//                        if (!stable[k]) {
                        float val = neighbor.second + tent[index];
                        if (tent[k] > val) {
                            if (tent[k] != numeric_limits<float>::max()) {
                                int b_index_old = (int) floor(tent[k] / delta);
                                bucket[b_index_old].erase(neighbor.first);
                            }
                            tent[k] = val;
                            int b_index_new = (int) floor(tent[k] / delta);
                            bucket[b_index_new].insert(neighbor.first);
                        }
                    }
                }
//                log << rank << " update req finished" << endl;
//                log.flush();
                MPI_Barrier(MPI_COMM_WORLD);
            }
            for(int j = 0; j < num_task; j++){
                size_heavy_receiver[j] = 0;
            }
            for(int j = i + 1; j < delta_length; j++){
                for (auto it_ = bucket[j].begin();
                     it_ != bucket[j].end(); it_++) {
                    int v = *it_;
                    float val = tent[v];
                    int v_host = v % num_task;
                    req_heavy_receiver[v_host][size_heavy_receiver[v_host]++] = make_pair(v, val);
                    if (size_heavy_receiver[v_host] >= capacity_req) {
                        cout << "light capacity is not enough " << size_heavy_receiver[v_host] << endl;
                        MPI_Finalize();
                    }
                }
                bucket[j].clear();
            }
            int rec_size = gather_data(req_heavy_receiver, size_heavy_receiver, req_heavy_sender, capacity_req, num_task, rank,
                                       option->message_length);
            for (int j = 0; j < rec_size; j++) {
                pair<int, float> neighbor = req_heavy_sender[j];
                int index = neighbor.first;

                float weight = neighbor.second;
                if (index % num_task != rank) {
                    cout << "there is not such vertex id : " << neighbor.first << " in " << "worker " << rank
                         << endl;
                    exit(0);
                }
                if (weight < tent[index]) {
                    if (tent[index] != numeric_limits<float>::max()) {
                        int b_index_old = (int) floor(tent[index] / delta);
                        bucket[b_index_old].erase(neighbor.first);
                    }
                    int b_index_new = (int)floor(weight / delta);
                    bucket[b_index_new].insert(neighbor.first);
//                            cout << rank << " : vertex id " << neighbor.first << " : tent index " << index
//                                 << " : bucket index " << b_index_new << endl;
                    tent[index] = weight;
                }
            }
        }
        clock_t end = clock();
        float time = (float) (end - start) / CLOCKS_PER_SEC;
        cout << rank << " used time " << time << endl;

        dump_result(option->output + to_string(rank), graph);

        MPI_Finalize();
    }

    void dump_result(string outpath, graph_dis *graph) {
        ofstream outfile(outpath);
        if (outfile) {
            cout << graph->size << endl;
            cout << tent.size() << endl;
            for (int i = 0; i < graph->size; i++) {
                outfile << graph->vertices[i]->id << " ";
                outfile << tent[graph->vertices[i]->id] << endl;
            }
        } else {
            cout << "not able to open file: " << outpath << endl;
        }

    }
};

#endif //SSSP_DELTA_AB_DIS_ORG_HPP
