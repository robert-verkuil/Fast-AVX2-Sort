#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <cstdlib>

#include "immintrin.h"


// Cache flushing, needed?
std::vector<int> flusher(1);
constexpr bool flush_cache = true;
void do_cache_flush() {
  for (auto x : flusher) {x++;}
}


// Timing code
#define TIME(func, repeats)                         \
do {                                                \
  int total_count = 0;                              \
  for (int i = 0; i < repeats; ++i) {               \
    do_cache_flush();                               \
    auto start = std::chrono::system_clock::now();  \
    func;                                           \
    auto end = std::chrono::system_clock::now();    \
    total_count += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(); \
  }                                                 \
  std::cout << "Avg Time taken = "                  \
            << total_count / repeats                \
            << " nanoseconds with "                 \
            << repeats << " repeats\n";             \
} while (0);                                        \

/////////////////////////////////////////////////////////////////////////////

// Intrinics
// Currently designed sort ints
__attribute__((always_inline)) inline
void sort_ints_intrinsics(const int *input_array, int *output_array) {
  __m256i input = _mm256_load_si256((__m256i *)input_array);
  __m256i result = _mm256_setzero_si256();
  for (int i = 0; i < 8; ++i) {
    __m256i b1 = (__m256i)_mm256_broadcast_ss((float *)(&input_array[i]));
    __m256i cmp = _mm256_cmpgt_epi32(input, b1);
    result = _mm256_sub_epi32(result, cmp);
  }
  __m256i output = _mm256_permutevar8x32_epi32(input, result);
  _mm256_store_si256((__m256i *)output_array, output);
  return;
}

// Inline asm
void sort_ints_asm(float *inputs) {
  __asm__ (
    // "vmovdqa64   (%1), %%ymm0\n"
    // "vpaddd      (%2), %%ymm0, %%ymm0\n"
    // "vmovdqa64   %%zmm0, (%0)"
    // "vpbroadcastd ymm0, xmmword PTR [inputs+0]\n"
    "vpbroadcastd ymm0, xmm1\n"

    // "vcmpgtd ymm, ymm, ymm\n"
    // "vpermps ymm, ymm, ymm\n"
    :
    : "r" (inputs)
    : "memory"
  );
  return;
}

int main(int argc, char *argv[]) {
  // if (cache_flush) {
  //   flusher.resize(64 * 1024 * 1024);
  // }
  auto inputs = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));
  for (int i = 0; i < 8; ++i) {
    inputs[i] = 7-i;
  }
  auto outputs = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));

  std::cout << "Intrinsic test\n";
  TIME(sort_ints_intrinsics(inputs, outputs), 10);
  for (int i = 0; i < 8; ++i) {
    std::cout << outputs[i] << ", ";
  }
  std::cout << "\n";

  std::cout << "ASM test\n";
  // sort_ints_asm(inputs);
  // TIME(sort_ints_asm(inputs), 10);
  return 0;
}
