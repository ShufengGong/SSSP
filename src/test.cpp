//
// Created by gongsf on 22/10/19.
//

#include "util.h"
#include <mpi.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    int rank;
    int num_task;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_task);

    int capacity = 1000;
    vector<pair<int, float> *> send_buffer = vector<pair<int, float> *>(num_task);
    int *send_size = new int[num_task];
    for(int i = 0; i < num_task; i++){
        send_buffer[i] = new pair<int, float>[capacity];
        send_size[i] = abs(rand() % 100);
    }

    ofstream log("logs/proc" + to_string(rank));
    if(!log.is_open()){
        cout << "no logs fold" << endl;
    }

    for(int i = 0; i < num_task; i++){
        for(int j = 0; j < send_size[i]; j++){
            send_buffer[i][j] = make_pair(i, j*1.0);
            log << send_buffer[i][j].first << ":" << send_buffer[i][j].second << "; ";
        }
        log << endl;
    }

    pair<int, float> *rec_buffer = new pair<int, float>[capacity];

    int rec_size = gather_data(send_buffer, send_size, rec_buffer, capacity, num_task, rank, 5);

    for(int i = 0; i < rec_size; i++){
        log << rec_buffer[i].first << ":" << rec_buffer[i].second << "; ";
    }
    log << endl;

    MPI_Finalize();
    return 0;
}

