#include <algorithm>
#include <assert.h>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "immintrin.h"

#define DEBUG


////////////////////////////////////////////////
// PRINTING
////////////////////////////////////////////////
#ifdef DEBUG
#define print_array(a,b)
#define print_ivec(a,b)
#else
void print_array(const int *array, std::string msg);
void print_ivec(__m256i vec, std::string msg);
#endif

////////////////////////////////////////////////
// TIMING
////////////////////////////////////////////////

class Timer {
public:
    typedef std::chrono::time_point<std::chrono::system_clock> time_pt_t;
    void start() { start_ = std::chrono::system_clock::now(); }
    void stop() { end_ = std::chrono::system_clock::now(); }
    int get_ns() { return std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();}
    void print() {
       int ns = get_ns();
       std::cout << "Avg Time taken = " << ns << " nanoseconds\n";
    }
private:
    time_pt_t start_, end_;
};
typedef std::function<void(int *, int *, int *)> sort_func_t;
void time_sort(sort_func_t sort_func, int *inputs, int *permutes, int *outputs);


////////////////////////////////////////////////
// CHECKING
////////////////////////////////////////////////

void check_sort(const int *inputs, const int *outputs);
void check_permute();
