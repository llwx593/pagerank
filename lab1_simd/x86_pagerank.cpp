#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <emmintrin.h>

using namespace std;

#define SHOW_FLAG 1

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

void avx2_page_rank() {
    for (int i = 0; i < NUM_ITER; i++) {
        struct timeval start = get_time();

        for (int j = 0; j < num_edges; j+=8) {
            __m256i s_point = _mm256_loadu_si256((__m256i*) &(graph.s_point[j]));
            __m256i e_point = _mm256_loadu_si256((__m256i*) &(graph.e_point[j]));

            __m256 vsum = _mm256_i32gather_ps(sum, e_point, 4);
            __m256 vrank = _mm256_i32gather_ps(rank_value, s_point, 4);
            __m256 voutbound = _mm256_i32gather_ps(graph.outbound, s_point, 4);

            vsum += vrank / voutbound;
            _mm256_i32scatter_ps(sum, e_point, vsum, 4);
        }

        for (int j = 0; j < num_nodes; j++) {
            rank_value[j] = (1 - damping_factor) / num_nodes + damping_factor * sum[j];
        }
        struct timeval end = get_time();
        cout << "One Itertion Time : ";
        cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl; 
    }

    cout << "AVX2 Version PageRank calculation completed" << endl;  
}

void avx512_page_rank() {
    for (int i = 0; i < NUM_ITER; i++) {
        struct timeval start = get_time();

        for (int j = 0; j < num_edges; j+=16) {
            void *src = &(graph.s_point[j]);
            __m512i s_point = _mm512_loadu_epi32(src);
            __m512i e_point = _mm512_loadu_epi32(&(graph.e_point[j]));
            __m512 vsum = _mm512_i32gather_ps(e_point, sum, 4);
            __m512 vrank = _mm512_i32gather_ps(s_point, rank_value, 4);
            __m512 voutbound = _mm512_i32gather_ps(s_point, graph.outbound, 4);

            vsum += vrank / voutbound;
            _mm512_i32scatter_ps(sum, e_point, vsum, 4);
        }

        for (int j = 0; j < num_nodes; j++) {
            rank_value[j] = (1 - damping_factor) / num_nodes + damping_factor * sum[j];
        }
        struct timeval end = get_time();
        cout << "One Itertion Time : ";
        cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl; 
    }

    cout << "AVX512 Version PageRank calculation completed" << endl;
}

void sse_page_rank() {
    for (int i = 0; i < NUM_ITER; i++) {
        struct timeval start = get_time();

        for (int j = 0; j < num_edges; j+=4) {
            __m128i s_point = _mm_loadu_si128((__m128i*) &(graph.s_point[j]));
            __m128i e_point = _mm_loadu_si128((__m128i*) &(graph.e_point[j]));
            int s1 = _mm_extract_epi32(s_point, 0);
            int s2 = _mm_extract_epi32(s_point, 1);
            int s3 = _mm_extract_epi32(s_point, 2);
            int s4 = _mm_extract_epi32(s_point, 3);
            int e1 = _mm_extract_epi32(e_point, 0);
            int e2 = _mm_extract_epi32(e_point, 1);
            int e3 = _mm_extract_epi32(e_point, 2);
            int e4 = _mm_extract_epi32(e_point, 3);
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

    cout << "SSE Version PageRank calculation completed" << endl;  
}

void show_value(int data_len) {
    cout << "=================== PageRank ===================" << endl;
    for (int i = 0; i < data_len; i++) {
        cout << "Node " << i << " PageRank is " << rank_value[i] << endl;
    }
    cout << "===================   Done   ===================" << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Error!!! Please input two args" << endl;
    } else {
        string file_name = argv[1];
        string simd_type = argv[2];
        get_graph(file_name);
        if (simd_type == "avx2")
            avx2_page_rank();
        else if (simd_type == "avx512")
            avx512_page_rank();
        else if (simd_type == "sse")
            sse_page_rank();
    }

    return 0;
}