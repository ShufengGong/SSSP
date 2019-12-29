//
// Created by gongsf on 16/3/19.
//

#ifndef SSSP_UTIL_H
#define SSSP_UTIL_H

#include <iostream>
#include <sstream>
#include <tbb/atomic.h>
#include <mpi.h>

#include "weight_graph.h"

using namespace std;

void set_weight(int argc, char *argv[]) {

    string input;
    string output;

    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--in") {
            input = argv[++i];
        }

        if (arg == "--out") {
            output = argv[++i];
        }
    }

    ofstream outfile(output.c_str());

    int max = 0;

    ifstream infile(input.c_str());
    if (infile.is_open()) {
        string line;
        for (int i = 0; getline(infile, line); i++) {

            int src;
            infile >> src;
            if (src > max) {
                max = src;
            }
            int des;
            infile >> des;
            if (des > max) {
                max = des;
            }
            float value = ((float) rand() / RAND_MAX) * 10;
            if (value <= 0) {
                cout << "0000" << endl;
                break;
            }
            outfile << src << "\t" << des << "," << value << endl;
        }
    } else {
        cout << "there is no file " << input << endl;
    }

    infile.close();

    outfile.close();
    cout << " max is " << max << endl;

}

void convert(int argc, char *argv[]) {

    string input;
    string output;
    int vertex_num = -1;

    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--in") {
            input = argv[++i];
        }

        if (arg == "--out") {
            output = argv[++i];
        }

        if (arg == "--vnum") {
            vertex_num = atoi(argv[++i]);
        }
    }

    weight_graph *graph = new weight_graph(vertex_num);
    graph->init_edge(input);
    graph->graph_dump(output);
}

void extract(int argc, char *argv[]) {
    string input;
    string output;
    int vertex_num = -1;
    int max_id = -1;
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--in") {
            input = argv[++i];
        }

        if (arg == "--out") {
            output = argv[++i];
        }

        if (arg == "--vnum") {
            vertex_num = atoi(argv[++i]);
        }

        if (arg == "--max_id") {
            max_id = atoi(argv[++i]);
        }
    }

    weight_graph *graph = new weight_graph(vertex_num);
    graph->init_vertex(input);
    graph->graph_extract_dump(output, max_id);
}

void atomic_update(tbb::atomic<float> &val, float y) noexcept {
    tbb::atomic<float> &pre_val = val;
    do {
//        cout << "tt" << endl;
        if (pre_val <= y) {
            break;
        }
    } while (val.compare_and_swap(y, pre_val) != pre_val);
}

int gather_data(vector<pair<int, float> *> &send_buffer, int send_size[], pair<int, float> *rec_buffer,
        int rec_capacity, int num_task, int rank, int message_length) {
    int receive_size;
    for (int i = 0; i < num_task; i++) {
        int *rec_count = new int[num_task];
        MPI_Allgather(&send_size[i], 1, MPI_INT, rec_count, 1, MPI_INT, MPI_COMM_WORLD);


        int max_rec = 0;
        int rec_size = 0;

        for (int j = 0; j < num_task; j++) {
            rec_size += rec_count[j];

            if (max_rec < rec_count[j]) {
                max_rec = rec_count[j];
            }
        }

        if (i == rank) {
            receive_size = rec_size;
        }

        if (rec_size >= rec_capacity) {
            cout << "the req receiver is not enough" << rec_size + 1 << endl;
            MPI_Finalize();
        }

        MPI_Barrier(MPI_COMM_WORLD);

        int iter_time = max_rec / message_length;
        if (max_rec % message_length != 0) {
            iter_time++;
        }
        int offset = 0;
        int *displs = new int[num_task];
        int *rec_count_size = new int[num_task];
        int *current_rec_count = new int[num_task];
        for (int j = 0; j < iter_time; j++) {
            int end = (j + 1) * message_length;

            int sum = 0;
            displs[0] = 0;
            if (rec_count[0] >= end) {
                current_rec_count[0] = message_length;
            } else if (rec_count[0] > (end - message_length)) {
                current_rec_count[0] = rec_count[0] - (end - message_length);
            } else {
                current_rec_count[0] = 0;
            }
            sum += current_rec_count[0];
            rec_count_size[0] = current_rec_count[0] * sizeof(pair<int, float>);
            for (int k = 1; k < num_task; k++) {
                displs[k] = displs[k - 1] + rec_count_size[k - 1];
                if (rec_count[k] >= end) {
                    current_rec_count[k] = message_length;
                } else if (rec_count[k] > (end - message_length)) {
                    current_rec_count[k] = rec_count[k] - (end - message_length);
                } else {
                    current_rec_count[k] = 0;
                }
                sum += current_rec_count[k];
                rec_count_size[k] = current_rec_count[k] * sizeof(pair<int, float>);
            }

            MPI_Gatherv(&send_buffer[i][j * message_length], current_rec_count[rank] * sizeof(pair<int, float>),
                        MPI_BYTE,
                        &rec_buffer[offset], rec_count_size, displs, MPI_BYTE, i, MPI_COMM_WORLD);
            offset += sum;
        }
        delete[] displs;
        delete[] rec_count_size;
        delete[] current_rec_count;
        delete[] rec_count;
    }
    return receive_size;
}

#endif //SSSP_DATASET_WEIGHT_H
