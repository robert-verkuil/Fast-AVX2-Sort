#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <cstdlib>

#include "immintrin.h"

// Timing code
#define TIME(func, repeats)                         \
do {                                                \
  for (int i = 0; i < repeats; ++i) {               \
    int total_count = 0;                            \
    auto start = std::chrono::system_clock::now();  \
    func();                                         \
    auto end = std::chrono::system_clock::now();    \
    total_count += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() \
  }                                                 \
  std::cout << "Time taken = "                      \
            << total_count                          \
            << " nanoseconds\n"                     \
} while (0);                                        \

// Cache flushing, needed?
std::vector<int> flusher(1);
constexpr bool cache_flush = false;
#define cache_flush() for (auto x : flusher) {x++;}

/////////////////////////////////////////////////////////////////////////////

// Intrinics
// Currently designed sort ints
void sort_ints_intrinsics(const int *input_array, int *output_array) {
  __m256i input = _mm256_load_si256(input_array);
  __m256i result = _mm256_setzero_si256(); // = _mm256_load_ps(output_array);
  for (int i = 0; i < 8; ++i) {
    __m256i b1 = (__m256i)_mm256_broadcast_ss((float *)&input_array[0]);
    __m256i cmp = _mm256_cmpgt_epi32(input, b1, 1); // _CMP_LT_OS
    __m256i icmp = _mm256_srli_epi32(cmp, 31);
    result = _mm256_add_epi32(result, icmp);
  }
  __m256i output = _mm256_permutevar8x32_epi32(input, result);
  _mm256_store_si256(output_array, output);
  return;
}

__attribute__((always_inline)) inline __m256i
_xmm256_and_si256(__m256i s1, __m256i s2)
{
    __m256i result;
    __asm__ ("vpand %2, %1, %0" : "=x"(result) : "x"(s1), "xm"(s2) );
    return result;
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
    inputs[i] = 7 - i;
  }
  auto outputs = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));

  // std::cout << "Intrinsic test\n";
  sort_ints_intrinsics(inputs, outputs);
  for (int i = 0; i < 8; ++i) {
    std::cout << outputs[i] << ", ";
  }
  std::cout << "\n";

  std::cout << "ASM test\n";
  // sort_ints_asm(inputs);
  // TIME(sort_ints_asm(inputs), 10);
  return 0;
}
