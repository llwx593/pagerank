#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

const int damping_factor = 0.85;
const int MAX_NODES = 2000000;
const int MAX_EDGES = 50000000;
const int NUM_ITER = 100;

int num_nodes, num_edges;
float rank_value[MAX_NODES];
float sum[MAX_NODES];

struct Web_Graph {
    int s_point[MAX_EDGES];
    int e_point[MAX_EDGES];
    int outbound[MAX_NODES];
};

Web_Graph graph;

struct timeval get_time() {
    strcut timeval now_time;
    gettimeofday(&now_time, NULL);
    return now_time;
}

void get_graph(string file_name) {
    ifstream file_in(file_name.c_str());

    string line;
    int count = 0;
    while(getline(file_in, line)) {
        stringstream s_in(line);
        if (count == 0) {
            s_in >> num_nodes >> num_edges;

            for (int i =0; i < num_nodes; i++) {
                rank_value[i] = 1.0;
                sum[i] = 0.0;
                graph.outbound[i] = 0;
            }

            count++;
        } else {
            int node1, node2;
            s_in >> node1 >> node2;
            graph.s_point[count-1] = node1;
            graph.e_point[count-1] = node2;
            graph.outbound[count-1]++;
            
            count++;
        }
    }

    cout << "Get the web graph" << endl;
}

void page_rank() {
    for (int i = 0; i < NUM_ITER; i++) {
        struct timeval start = get_time();

        for (int j = 0; j < num_edges; j++) {
            int s_point = graph.s_point[j];
            int e_point = graph.e_point[j];

            sum[e_point] += rank_value[s_point] / graph.outbound[s_point];
        }

        for (int j = 0; j < num_nodes; j++) {
            rank_value[j] = (1 - damping_factor) / num_nodes + damping_factor * sum[j];
        }

        struct timeval end = get_time();
        cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl; 
    }

    cout << "PageRank calculation completed" << endl;
}

void show_value() {
    cout << "=================== PageRank ===================" << endl;
    for (int i = 0; i < num_nodes; i++) {
        cout << "Node " << i << " PageRank is " << rank_value[i] << endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Error!!! Should have filename" << endl;
    } else {
        char *file_name = argv[1];
        get_graph(file_name);
        page_rank();
        show_value();
    }

    return 0;
}