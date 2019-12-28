//
// Created by gongsf on 12/18/19.
//

#ifndef SRC_SSSP_BELLMAN_FORD_HPP
#define SRC_SSSP_BELLMAN_FORD_HPP

#include<iostream>
#include<mpi.h>
#include <vector>
#include <limits>

#include "sssp_option.hpp"
#include "util.h"
#include "graph_dis.hpp"

using namespace std;
using namespace sssp;

struct bellman_ford {

public:
    sssp_option *option;
    int num_task;
    int rank;
    vector<float> tent;
    vector<int> v_iter;


    bellman_ford(sssp_option *option_){
        option = option_;
        if (option->num_vertex == -1 || option->source == -1 || option->req_capacity == -1 || option->delta == -1 ||
            option->delta_length == -1 || option->iter_time == -1) {
            cout << "invalid paramete!" << endl;
            exit(0);
        }
    }
    void run(int *argc, char ***argv) {
        cout << "init MPI" << endl;
        MPI_Init(argc, argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_task);
        int num_vertex = option->num_vertex / num_task;
        int capacity = option->req_capacity;
        graph_dis *graph = new graph_dis(num_vertex);
        graph->init(option->input, rank, num_task);
        tent = vector<float>(num_vertex, numeric_limits<float>::max());
        if(option->source % num_task == rank){
            tent[option->source / num_task] = 0;
        }
        int *size_sender = new int[num_task];
        for (int j = 0; j < num_task; j++) {
            size_sender[j] = 0;
        }

        vector<pair<int, float> *> sender = vector <pair< int, float > *> (num_task);//发送缓存区
        pair<int, float> *receiver = new pair<int, float>[capacity];//接收
        for (int i = 0; i < num_task; i++) {
            sender[i] = new pair<int, float>[capacity];
        }

        int size = graph->size;
        int iter_time = option->iter_time;
        clock_t start = clock();
        for (int i = 0; i < iter_time; i++) {
            for (int i = 0; i < size; i++) {
                vector<pair<int, float>> &neighbor = graph->vertices[i]->neighbor;

                for (int j = 0; j < neighbor.size(); j++) {
                    int k = neighbor[j].first;
                    float val = neighbor[j].second + tent[i];
                    int k_host = k % num_task;
                    sender[k_host][size_sender[k_host]++] = make_pair(k, val);
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
            //获取接收到的图数据的大小
            int *recv_count = new int[num_task];
            for (int j = 0; j < num_task; j++) {
                MPI_Gather(&size_sender[j], 1, MPI_INT, recv_count, 1, MPI_INT, j, MPI_COMM_WORLD);
            }
            //偏移量
            int *displs = new int[num_task];
            displs[0] = 0;
            int rec_size = recv_count[0];
            recv_count[0] *= sizeof(pair < int, float >);
            for (int j = 1; j < num_task; j++) {
                rec_size += recv_count[j];
                recv_count[j] *= sizeof(pair < int, float >);
                displs[j] = displs[j - 1] + recv_count[j - 1];
            }
            MPI_Barrier(MPI_COMM_WORLD);
            //发送数据
            if(rec_size >= capacity){
                capacity = rec_size + 1;
                receiver = new pair<int, float>[capacity];
            }
            for (int j = 0; j < num_task; j++) {
                MPI_Gatherv(sender[j], size_sender[j] * sizeof(pair < int, float >), MPI_BYTE,
                            receiver, recv_count, displs, MPI_BYTE, j, MPI_COMM_WORLD);

            }
            if (rec_size != 0) {
                for (int j = 0; j < rec_size; j++) {
                    pair<int, float> neighbor = receiver[j];
                    int index = neighbor.first / num_task;
                    float weight = neighbor.second;
                    if (index % num_task != rank) {
                        cout << "there is not such vertex id : " << neighbor.first << " in " << "worker " << rank
                             << endl;
                        exit(0);
                    }
                    if (weight < tent[index/num_task]) {
                        tent[index/num_task] = weight;
                    }
                }
            }
        }
        clock_t end = clock();
        float time = (float) (end - start) / CLOCKS_PER_SEC;
        cout << rank << " used time " << time << endl;
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


#endif //SRC_SSSP_BELLMAN_FORD_HPP
