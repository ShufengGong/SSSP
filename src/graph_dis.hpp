//
// Created by gongsf on 12/2/19.
//

#ifndef SSSP_GRAPH_DIS_HPP
#define SSSP_GRAPH_DIS_HPP

#include <vector>
#include <fstream>

struct vertex{
public:
    int id;
    vector<pair<int, float>> neighbor;

    explicit vertex(int id_){
        id = id_;
        neighbor.clear();
    }

    ~vertex(){
        vector<pair<int, float>>().swap(neighbor);
    }
};

struct graph_dis{
public:
    int num_vertex;
    int size;
    vector<vertex *> vertices;
    explicit graph_dis(int num_vertex_): num_vertex(num_vertex_){
        size = 0;
        if(num_vertex > 0) {
            vertices = vector<vertex *>(num_vertex);
        }else{
            cout << "graph init error, the number of vertices < 0" << endl;
        }
    }

    void init(string input, int rank, int num_task){
        ifstream infile(input);
        if (infile.is_open()) {
            string str;
            while (getline(infile, str)) {
                boost::trim(str);
                int pos = str.find_first_of("\t");
                int src = atoi(str.substr(0, pos).c_str());
                if(src % num_task != rank){
                    cout << "the vertex id is not correspond to task id" << endl;
                }
                vertex *vertex_ = new vertex(src);
                string remain = str.substr(pos + 1);
                if (*remain.end() != ' ') {
                    remain += " ";
                }
                while ((pos = remain.find_first_of(" ")) != remain.npos) {
                    int target;
                    float weight;
                    if (pos > 0) {
                        string cur = remain.substr(0, pos);
                        int cut = cur.find_first_of(",");
                        target = atoi(cur.substr(0, cut).c_str());
                        weight = atof(cur.substr(cut + 1).c_str());
                        vertex_->neighbor.emplace_back(make_pair(target, weight));
                    }
                    remain = remain.substr(pos + 1);
                }
                vertices[src / num_task] = vertex_;
                size++;
            }
            cout << "init finished" << endl;
        }
    }

//    void dump(string outpath){
//        ofstream outfile(outpath);
//        if(outfile){
//            for(int i = 0; i < size; i++){
//                outfile << vertices[i]->id << " ";
//                vector<pair<int, float>> &neighbor = vertices[i]->neighbor;
//                for(int j = 0; j < neighbor.size(); j++){
//                    outfile << (neighbor)[j].first << "," << (neighbor)[j].second << " ";
//                }
//                cout << endl;
//            }
//        }else{
//            cout << "can not open such file : " << outpath << endl;
//        }
//
//    }

    void display(){
        for(int i = 0; i < size; i++){
            vector<pair<int, float>> &neighbor = vertices[i]->neighbor;
            cout << &neighbor << endl;
            cout << &(vertices[i]->neighbor) << endl;
            cout << vertices[i]->id << "\t";

            for(int j = 0; j < neighbor.size(); j++){
                cout << (neighbor)[j].first << "," << (neighbor)[j].second << " ";
            }
            cout << endl;
        }
    }

    ~graph_dis(){
        vector<vertex *>().swap(vertices);
    }
};

#endif //SSSP_GRAPH_DIS_HPP
