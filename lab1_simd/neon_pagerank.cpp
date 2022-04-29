#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>
#include <arm_neon.h>

using namespace std;

const float damping_factor = 0.85;
const int MAX_NODES = 1500000;
const int MAX_EDGES = 30000000;
const int NUM_ITER = 10;

int num_nodes, num_edges;
float rank_value[MAX_NODES];
float sum[MAX_NODES];

struct Web_Graph {
    int s_point[MAX_EDGES];
    int e_point[MAX_EDGES];
    float outbound[MAX_NODES];
};

Web_Graph graph;

struct timeval get_time() {
    struct timeval now_time;
    gettimeofday(&now_time, NULL);
    return now_time;
}

void get_graph(string file_name) {
    ifstream file_in(file_name.c_str());
    string line;
    getline(file_in, line);
    stringstream sin(line);
    sin >> num_nodes >> num_edges;

    for (int i = 0; i < num_nodes; i++) {
        rank_value[i] = 1.0;
        sum[i] = 0.0;
        graph.outbound[i] = 0.0;
    }

    int cur = 0;
    while(getline(file_in, line)) {
        int n1, n2;
        stringstream sin1(line);
        sin1 >> n1 >> n2;
        graph.s_point[cur] = n1;
        graph.e_point[cur] = n2;
        graph.outbound[n1] += 1.0;
        cur++;
    }
    num_edges = cur;

    cout << "Get the web graph" << endl;
}

void neon_page_rank() {
    for (int i = 0; i < NUM_ITER; i++) {
        struct timeval start = get_time();

        for (int j = 0; j < num_edges; j+=4) {
            int32x4_t s_point = vld1q_s32(&(graph.s_point[j]));
            int32x4_t e_point = vld1q_s32(&(graph.e_point[j]));
            
            int32_t s1 = vgetq_lane_s32(s_point, 0);
            int32_t s2 = vgetq_lane_s32(s_point, 1);
            int32_t s3 = vgetq_lane_s32(s_point, 2);
            int32_t s4 = vgetq_lane_s32(s_point, 3);
            int32_t e1 = vgetq_lane_s32(e_point, 0);
            int32_t e2 = vgetq_lane_s32(e_point, 1);
            int32_t e3 = vgetq_lane_s32(e_point, 2);
            int32_t e4 = vgetq_lane_s32(e_point, 3);
            sum[e1] = rank_value[s1] / graph.outbound[s1];
            sum[e2] = rank_value[s2] / graph.outbound[s2];
            sum[e3] = rank_value[s3] / graph.outbound[s3];
            sum[e4] = rank_value[s4] / graph.outbound[s4];
        }

        for (int j = 0; j < num_nodes; j++) {
            rank_value[j] = (1 - damping_factor) / num_nodes + damping_factor * sum[j];
        }
        struct timeval end = get_time();
        cout << "One Itertion Time : ";
        cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl; 
    }

    cout << "Neon Version PageRank calculation completed" << endl;      
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Error!!! Should have filename" << endl;
    } else {
        char *file_name = argv[1];
        get_graph(file_name);
        neon_page_rank();
    }

    return 0;
}