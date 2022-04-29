### PageRank
#### Introduction
This is the experiment of parallel computing course.

avx512 command is :
g++ -O2 -march=native -std=c++11 -mavx512f -o simd_pagerank simd_pagerank.cpp

avx2 command is :
g++ -O2 -march=native -std=c++11 -mavx2 -o simd_pagerank simd_pagerank.cpp

sse2 command is :
g++ -O2 -march=native -std=c++11 -msse2 -o simd_pagerank simd_pagerank.cpp

neon command is :
g++ -g -march=native arm_pagerank.cpp -o arm_pagerank

