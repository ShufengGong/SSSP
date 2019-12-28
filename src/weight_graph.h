//
// Created by gongsf on 14/3/19.
//

#ifndef SSSP_WEIGHT_GRAPH_H
#define SSSP_WEIGHT_GRAPH_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace std;

class weight_graph {

public:

    int vertex_num;
    vector<vector<pair<int, float>>> g;
    vector<float> dis;
    vector<int> v_iter;

    weight_graph(int n) {
        vertex_num = n;
    }

    void init_vertex(string path){
        g = vector<vector<pair<int, float>>>(vertex_num, vector<pair<int, float>>());
        ifstream infile(path.c_str());
        if (infile.is_open()) {
            string str;
            while (getline(infile, str)) {
                boost::trim(str);
                int pos = str.find_first_of("\t");
                int src = atoi(str.substr(0, pos).c_str());

                string remain = str.substr(pos + 1);
                vector<pair<int, float>> neighbor;
                if (*remain.end() != ' ') {
                    remain += " ";
                }
                while((pos = remain.find_first_of(" ")) != remain.npos){
                    int target;
                    float weight;
                    if(pos > 0){
                        string cur = remain.substr(0, pos);
                        int cut = cur.find_first_of(",");
                        target = atoi(cur.substr(0, cut).c_str());
                        weight = atof(cur.substr(cut + 1).c_str());
                        neighbor.push_back(make_pair(target, weight));
                    }
                    remain = remain.substr(pos + 1);
                }
                g[src] = neighbor;
            }

        } else {
            cout << "there is no file " << path << endl;
        }
        cout << "init graph finished" << endl;
        infile.close();
    }

    void init_edge(string path) {
        g = vector<vector<pair<int, float>>>(vertex_num, vector<pair<int, float>>());
        ifstream infile(path.c_str());
        if (infile.is_open()) {
            string str;
            while (getline(infile, str)) {
                boost::trim(str);
                int pos = str.find_first_of("\t");
                int src = atoi(str.substr(0, pos).c_str());

                string remain = str.substr(pos + 1);
                int cut = remain.find_first_of(",");
                int target = atoi(remain.substr(0, cut).c_str());
                float weight = atof(remain.substr(cut + 1).c_str());
                g[src].push_back(make_pair(target, weight));
            }

        } else {
            cout << "there is no file " << path << endl;
        }
        cout << "init graph finished" << endl;
        infile.close();
    }

    void set_dis(vector<float> dis_) {
        dis = dis_;
    }

    void set_v_iter(vector<int> v_iter_) {
        v_iter = v_iter_;
    }

    void dump_result(string output) {
        cout << "start dump" << endl;
        ofstream outfile(output.c_str());
        for (int i = 0; i < vertex_num; i++) {
            outfile << i << "\t" << v_iter[i] << "\t";
            outfile << dis[i] << endl;
        }
        outfile.close();
    }

    void graph_dump(string output) {
        ofstream outfile(output.c_str());
        for (int i = 0; i < vertex_num; i++) {
            outfile << i << "\t";
            vector<pair<int, float>> neighbors = g[i];
            if (neighbors.empty()) {
                if (i == 0) {
                    outfile << 1 << "," << (float) ((rand() / RAND_MAX) * 10);
                } else {
                    outfile << i - 1 << "," << (float) ((rand() / RAND_MAX) * 10);
                }
            } else {
                for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                    pair<int, float> neighbor = *it;
                    outfile << neighbor.first << "," << neighbor.second << " ";
                }
            }
            outfile << endl;
        }
        outfile.clear();
    }

    void graph_extract_dump(string output, int max_id) {
        ofstream outfile(output.c_str());
        for (int i = 0; i < vertex_num; i++) {
            if(i > max_id){
                break;
            }
            outfile << i << "\t";
            vector<pair<int, float>> neighbors = g[i];
            if (neighbors.empty()) {
                if (i == 0) {
                    outfile << 1 << "," << (float) rand() / RAND_MAX;
                } else {
                    outfile << i - 1 << "," << (float) rand() / RAND_MAX;
                }
            } else {
                for (vector<pair<int, float>>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                    pair<int, float> neighbor = *it;
                    if(neighbor.first <= max_id){
                        outfile << neighbor.first << "," << neighbor.second << " ";
                    }
                }
            }
            outfile << endl;
        }
        outfile.clear();
    }

    ~weight_graph() {
        g.clear();
        vertex_num = 0;
    }
};

#endif //SSSP_WEIGHT_GRAPH_H
