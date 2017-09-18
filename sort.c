#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

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
void sort_ints_intrinsics(__m256i packed_input) {
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
void sort_ints_asm(int *inputs) {
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
  if (cache_flush) {
    flusher.resize(64 * 1024 * 1024);
  }

  std::cout << "Intrinsic test\n";
  // TIME(sort_ints_intrinsics(__m256i packed_input)); //TODO

  std::cout << "ASM test\n";
  int inputs[8] = {7,6,5,4,3,2,1,0};
  sort_ints_asm(inputs);
  // TIME(sort_ints_asm(inputs), 10);
  return 0;
}
