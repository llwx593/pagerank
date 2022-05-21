#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

using namespace std;

#define SHOW_FLAG 1

const float damping_factor = 0.85;
const int MAX_NODES = 1500000;
const int MAX_EDGES = 30000000;
const int NUM_ITER = 10;

int num_nodes, num_edges, num_threads;
float rank_value[MAX_NODES];
float sum[MAX_NODES];

struct Web_Graph {
    int s_point[MAX_EDGES];
    int e_point[MAX_EDGES];
    int outbound[MAX_NODES];
};

struct Thread
{
	int tid;
	int start, end;
    int start_n, end_n;
};

struct timeval get_time() {
    struct timeval now_time;
    gettimeofday(&now_time, NULL);
    return now_time;
}

Web_Graph graph;
pthread_t *Threads;
Thread *Threads_data;

void get_graph(string file_name) {
    ifstream file_in(file_name.c_str());
    string line;
    getline(file_in, line);
    stringstream sin(line);
    sin >> num_nodes >> num_edges;

    for (int i = 0; i < num_nodes; i++) {
        rank_value[i] = 1.0;
        sum[i] = 0.0;
        graph.outbound[i] = 0;
    }

    int cur = 0;
    while(getline(file_in, line)) {
        int n1, n2;
        stringstream sin1(line);
        sin1 >> n1 >> n2;
        graph.s_point[cur] = n1;
        graph.e_point[cur] = n2;
        graph.outbound[n1]++;
        cur++;
    }
    num_edges = cur;

    cout << "Get the web graph" << endl;
}

void thread_allocation() {
	double E_split = (double) num_edges / num_threads;
	double N_split = (double) num_nodes / num_threads;
	Threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));		
	Threads_data = (Thread*)malloc(num_threads * sizeof(Thread));	
	
	Threads_data[0].tid = 0;
	Threads_data[0].start = 0;
	Threads_data[0].end = floor(E_split);
    Threads_data[0].start_n = 0;
    Threads_data[0].end_n = floor(N_split);

	for (int i = 1; i < num_threads; i++) {
		Threads_data[i].tid = i;
		Threads_data[i].start = Threads_data[i - 1].end;
        Threads_data[i].start_n = Threads_data[i - 1].end_n;
		if (i < (num_threads - 1)) {
			Threads_data[i].end = Threads_data[i].start + floor(E_split);
            Threads_data[i].end_n = Threads_data[i].start_n + floor(N_split);
		}
		else {
			Threads_data[i].end = num_edges;
            Threads_data[i].end_n = num_nodes;
		}
	}
	
	printf("\n");

	for (int i = 0; i < num_threads; i++) {
		printf("Thread %d, start = %d, end = %d", Threads_data[i].tid, Threads_data[i].start, Threads_data[i].end);
        printf(", start_n = %d, end_n = %d\n", Threads_data[i].start_n, Threads_data[i].end_n);
	}

	printf("\n");
}

void *compute_sum(void *arg) {
	Thread *thread_data = (Thread *) arg;
	
	for (int i = thread_data->start; i < thread_data->end; i++) {
        int s_point = graph.s_point[i];
        int e_point = graph.e_point[i];

        sum[e_point] += rank_value[s_point] / graph.outbound[s_point];
	}
}

void *compute_rank(void *arg) {
    Thread *thread_data = (Thread *) arg;

    for (int i = thread_data->start_n; i < thread_data->end_n; i++) {
        rank_value[i] = (1 - damping_factor) / num_nodes + damping_factor * sum[i];
    }
}

void page_rank() {
    double avg_time = 0.0;
    
    for (int i = 0; i < NUM_ITER; i++) {
        struct timeval start = get_time();

		for (int j = 0; j < num_threads; j++) {
			pthread_create(&Threads[j], NULL, &compute_sum, (void *)&Threads_data[j]);
		}

        for (int j = 0; j < num_threads; j++) {
            pthread_join(Threads[j], NULL);
        }

        for (int j = 0; j < num_threads; j++) {
            pthread_create(&Threads[j], NULL, &compute_rank, (void *)&Threads_data[j]);
        }

        for (int j = 0; j < num_threads; j++) {
            pthread_join(Threads[j], NULL);
        }
        struct timeval end = get_time();
        avg_time += (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        cout << "One Itertion Time : ";
        cout << (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0 << endl; 
    }

    avg_time /= NUM_ITER;
    cout << "Average Time : ";
    cout << avg_time << endl;
    cout << "PageRank calculation completed" << endl;
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
        cout << "Error!!! Should have filename and the number of thread" << endl;
    } else {
        char *file_name = argv[1];
		num_threads = atoi(argv[2]);
        get_graph(file_name);
        thread_allocation();
        page_rank();
        if (SHOW_FLAG) {
            int data_len = 10;
            show_value(data_len);
        }
    }

    return 0;
}