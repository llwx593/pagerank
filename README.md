### PageRank
#### Introduction
This is the experiment of parallel computing course.

x86 command is :
g++ -O2 -march=native -std=c++11 -mavx2 -mavx512f -msse2 -o x86_pagerank x86_pagerank.cpp

neon command is :
g++ -g -march=native neon_pagerank.cpp -o neon_pagerank

