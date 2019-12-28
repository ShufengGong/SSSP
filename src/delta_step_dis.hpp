//
// Created by gongsf on 11/27/19.
//

#ifndef SSSP_DELTA_STEP_DIS_HPP
#define SSSP_DELTA_STEP_DIS_HPP

#include "sssp_option.hpp"
#include "util.h"
#include "graph_dis.hpp"

#include <mpi.h>
#include <vector>
#include <unordered_set>

using namespace std;
using namespace sssp;

struct delta_step_dis {
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

    delta_step_dis(sssp_option *option_) {
        option = option_;

        if (option->num_vertex == -1 || option->source == -1 || option->req_capacity == -1 || option->delta == -1 ||
            option->delta_length == -1) {
            cout << "invalid paramete!" << endl;
            exit(0);
        }

    }

    int run(int *argc, char ***argv) {
        cout << "init MPI" << endl;
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_task);
        int num_vertex = option->num_vertex / num_task;
        float delta = option->delta;
        int delta_length = option->delta_length;
        int capacity = option->req_capacity;
        int light_capacity = capacity;
        int heavy_receiver_capacity = capacity;
        int *size_light_receiver = new int[num_task];
        int *size_heavy_receiver = new int[num_task];
//        int size_light_sender = 0;
//        int size_heavy_sender = 0;
        graph_dis *graph = new graph_dis(num_vertex);
        cout << rank << " loading graph" << endl;
        graph->init(option->input + "/part" + to_string(rank), rank, num_task);
        cout << rank << " load garph finished" << endl;

        vector<pair<int, float> *> req_light_receiver = vector<pair<int, float> *>(num_task);
        vector<pair<int, float> *> req_heavy_receiver = vector<pair<int, float> *>(num_task);
        pair<int, float> *req_light_sender = new pair<int, float>[capacity];
        pair<int, float> *req_heavy_sender = new pair<int, float>[capacity];
        for (int i = 0; i < num_task; i++) {
            req_light_receiver[i] = new pair<int, float>[light_capacity];
            size_light_receiver[i] = 0;
            req_heavy_receiver[i] = new pair<int, float>[heavy_receiver_capacity];
            size_heavy_receiver[i] = 0;
        }
        tent = vector<float>(num_vertex, numeric_limits<float>::max());
        v_iter = vector<int>(num_vertex, 0);
        vector<unordered_set<int>> bucket = vector<unordered_set<int>>(option->delta_length, unordered_set<int>());

        MPI_Barrier(MPI_COMM_WORLD);

        int source = option->source;
        if (source % num_task == rank) {
            tent[source / num_task] = 0;
            v_iter[source / num_task]++;
            bucket[0].insert(source);
        }

        ofstream log("logs/proc" + to_string(rank));

        clock_t start = clock();
        for (int i = 0; i < delta_length; i++) {
//            log << rank << " iter " << i << endl;
//            cout << rank << " iter " << i << endl;
            MPI_Barrier(MPI_COMM_WORLD);
            if (is_all_empty(bucket[i].size())) {
                continue;
            }
            for (int j = 0; j < num_task; j++) {
                size_heavy_receiver[j] = 0;
            }
            while (!is_all_empty(bucket[i].size())) {
                for (int j = 0; j < num_task; j++) {
                    size_light_receiver[j] = 0;
                }
                if (!bucket[i].empty()) {
                    for (auto it_ = bucket[i].begin();
                         it_ != bucket[i].end(); it_++) {
                        int v = *it_ / num_task;
                        if (*it_ % num_task != rank) {
                            cout << "the vertex id in wrong bucket" << endl;
                            exit(0);
                        }
                        vector<pair<int, float>> &neighbors = (graph->vertices[v]->neighbor);

                        for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
                            pair<int, float> neighbor = *it;
                            if (neighbor.second < delta) {
                                int k = neighbor.first;
                                float val = neighbor.second + tent[v];
                                int k_host = k % num_task;
                                req_light_receiver[k_host][size_light_receiver[k_host]++] = make_pair(k, val);
                                if (size_light_receiver[k_host] >= light_capacity) {
                                    cout << "light capacity is not enough " << size_light_receiver[k_host] << endl;
                                    exit(0);
                                }
                            } else {
                                int k = neighbor.first;
                                float val = neighbor.second + tent[v];
                                int k_host = k % num_task;
                                req_heavy_receiver[k_host][size_heavy_receiver[k_host]++] = make_pair(k,
                                                                                                      val);
                                if (size_heavy_receiver[k_host] >= heavy_receiver_capacity) {
                                    cout << "heavy capacity is not enough " << size_heavy_receiver[k_host] << endl;
                                    exit(0);
                                }
                            }
                        }
                    }
                    bucket[i].clear();
                }
//                log << rank << " send light req" << endl;
//                log.flush();
//                MPI_Barrier(MPI_COMM_WORLD);
                //send and receive light req
                int rec_size = gather_data(req_light_receiver, size_light_receiver, req_light_sender, capacity,
                                           num_task, rank, option->message_length);
//                int *recv_count = new int[num_task];
//                for (int j = 0; j < num_task; j++) {
//                    MPI_Gather(&size_light_receiver[j], 1, MPI_INT, recv_count, 1, MPI_INT, j, MPI_COMM_WORLD);
//                }
//
//                int *displs = new int[num_task];
//                displs[0] = 0;
//                int rec_size = recv_count[0];
//                recv_count[0] *= sizeof(pair<int, float>);
//                for (int j = 1; j < num_task; j++) {
//                    rec_size += recv_count[j];
//                    recv_count[j] *= sizeof(pair<int, float>);
//                    displs[j] = displs[j - 1] + recv_count[j - 1];
//                }
////                log << rank << " receive light req" << endl;
////                log.flush();
//                if(rec_size >= capacity){
//                    capacity = rec_size + 1;
//                    delete[] req_light_sender;
//                    req_light_sender = new pair<int, float>[capacity];
//                }
//                MPI_Barrier(MPI_COMM_WORLD);
//                for (int j = 0; j < num_task; j++) {
////                    if (rec_size != 0) {
//                    MPI_Gatherv(req_light_receiver[j], size_light_receiver[j] * sizeof(pair<int, float>), MPI_BYTE,
//                                req_light_sender, recv_count, displs, MPI_BYTE, j, MPI_COMM_WORLD);
////                    }
//                }
//                log << rank << " receivedddd light req " << endl;
//                log.flush();
//                MPI_Barrier(MPI_COMM_WORLD);
                if (rec_size != 0) {
                    for (int j = 0; j < rec_size; j++) {
                        pair<int, float> neighbor = req_light_sender[j];
                        int index = neighbor.first / num_task;
                        float weight = neighbor.second;
                        if (neighbor.first % num_task != rank) {
                            cout << "there is not such vertex id : " << neighbor.first << " in " << "worker " << rank
                                 << endl;
                            exit(0);
                        }
                        if (weight < tent[index]) {
                            int b_index_new = (int) floor(weight / delta);
                            if (tent[index] != numeric_limits<float>::max()) {
                                int b_index_old = (int) floor(tent[index] / delta);
                                if (b_index_old != b_index_new) {
                                    bucket[b_index_old].erase(neighbor.first);
                                }
                            }
                            bucket[b_index_new].insert(neighbor.first);
//                            cout << rank << " : vertex id " << neighbor.first << " : tent index " << index
//                                 << " : bucket index " << b_index_new << endl;
                            tent[index] = weight;
                            v_iter[index]++;
                        }
                    }
                }
//                log << rank << " update light req finished" << endl;
//                log.flush();
                MPI_Barrier(MPI_COMM_WORLD);
            }
            //send and receive heavy req
            int rec_size = gather_data(req_heavy_receiver, size_heavy_receiver, req_heavy_sender, capacity, num_task,
                                       rank, option->message_length);

            if (rec_size != 0) {
                for (int j = 0; j < rec_size; j++) {
                    pair<int, float> neighbor = req_heavy_sender[j];
                    int index = neighbor.first / num_task;
                    float weight = neighbor.second;
                    if (neighbor.first % num_task != rank) {
                        cout << "there is not such vertex id : " << neighbor.first << " in " << "worker " << rank
                             << endl;
                        exit(0);
                    }
                    if (weight < tent[index]) {
                        int b_index_new = (int) floor(weight / delta);
                        if (tent[index] != numeric_limits<float>::max()) {
                            int b_index_old = (int) floor(tent[index] / delta);
                            if (b_index_old != b_index_new) {
                                bucket[b_index_old].erase(neighbor.first);
                            }
                        }
//                        cout << rank << " : vertex id " << neighbor.first << " : tent index " << index
//                             << " : bucket index " << b_index_new << endl;
                        bucket[b_index_new].insert(neighbor.first);
                        tent[index] = weight;
                        v_iter[index]++;
                    }
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
                outfile << tent[i] << endl;
            }
        } else {
            cout << "not able to open file: " << outpath << endl;
        }

    }

};

#endif //SSSP_DELTA_STEP_DIS_HPP
