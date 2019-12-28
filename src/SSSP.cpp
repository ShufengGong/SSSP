#include <iostream>
#include <vector>
#include "delta_simple.h"
#include "util.h"
#include "delta_priority.h"
#include "dijkstra.h"


using namespace std;

void test(){
    vector<int> v1;
    vector<int> v2;
    v1.push_back(1);
    v1.push_back(2);
    v2 = v1;
    v1.clear();
    cout << v1.size() << endl;
    cout << v2.size() << endl;
    cout << v2[1] << endl;
}

int main(int argc, char *argv[]) {

    string fun;

//    MutablePriorityQueue<int, float> q = MutablePriorityQueue<int, float>(50);
//    q.Insert(1, 0.8);
//    q.Insert(2, 1.3);
//    q.Insert(3, 0.1);
//    q.Insert(4, 4.2);
//    q.Insert(5, 2.1);
//    cout << q.Peek().pri << endl;
//    q.Pop();
//    q.ChangePriority(5, 0.2);
//    cout << q.Peek().pri << endl;

    for (int i = 0; i < argc; i++) {
        string arg = argv[i];

        if(arg == "--fun"){
            fun = argv[++i];
        }
    }

    if (fun == "set_weight") {
        set_weight(argc, argv);
    }else  if (fun == "delta_simple") {
        delta_simple(argc, argv);
    }else if(fun == "delta_priority") {
        delta_priority(argc, argv);
    }else if(fun == "convert_graph"){
        convert(argc, argv);
    }else if(fun == "dijkstra"){
        dijkstra(argc, argv);
    } else if(fun == "extract") {
        extract(argc, argv);
    }else{
            cout << "there is no function" << fun << endl;
    }

    return 0;
}