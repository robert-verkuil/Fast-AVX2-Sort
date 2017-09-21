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

#include "util.h"

// Cache flushing
constexpr bool FLUSH_CACHE = true;
constexpr int CACHE_SIZE = 16 * 1024 * 1024; // 64MB
std::vector<int> flusher(1);
void do_cache_flush() {
  if (flusher.size() == 1 && FLUSH_CACHE) {
    flusher.resize(CACHE_SIZE);
  }
  for (auto x : flusher) {x++;}
}


// Intrinics
__attribute__((always_inline)) inline
void sort_ints_intrinsics(int *input_array, int *permute_array, int *output_array) {
  __m256i orig_input = _mm256_load_si256((__m256i *)input_array);
  __m256i shifted_input = _mm256_load_si256((__m256i *)input_array);
  __m256i permute = _mm256_load_si256((__m256i *)permute_array);
  __m256i accumulate = _mm256_setzero_si256();

  print_ivec(orig_input, "ivec orig_input");
  print_ivec(shifted_input, "ivec shifted_input");
  print_ivec(permute, "ivec permute");
  print_ivec(accumulate, "ivec accumulate");

  for (int i = 0; i < 7; ++i) {
    shifted_input = _mm256_permutevar8x32_epi32(shifted_input, permute);
    __m256i cmp = _mm256_cmpgt_epi32(orig_input, shifted_input);
    accumulate = _mm256_sub_epi32(accumulate, cmp);
  }
  print_ivec(accumulate, "ivec accumulate");
  __m256i output = _mm256_permutevar8x32_epi32(orig_input, accumulate);
  print_ivec(output, "ivec output");

  _mm256_store_si256((__m256i *)output_array, output);
  return;
}


// Inline asm
void sort_ints_asm(const int *input_array, int *permute_array, int *output_array) {
  __asm__ __volatile__ (
    // "mov %[input_array], rax\t\n"
    // "mov %[permute_array], rbx\t\n"
    // "mov %[output_array], rcx\t\n"
    "mov rax, %[input_array]\t\n"
    "mov rbx, %[permute_array]\t\n"
    "mov rcx, %[output_array]\t\n"
    "mov r12, 7\t\n"

    "vmovdqu ymm0, YMMWORD PTR [rax]\t\n"    // ymm0 = orig_input
    "vmovdqu ymm1, YMMWORD PTR [rax]\t\n"    // ymm1 = shifted_input
    "vmovdqu ymm2, YMMWORD PTR [rbx]\t\n"   // ymm2 = perm
    "vpxor ymm3, ymm3, ymm3\t\n"            // ymm3 = accumulate
                                            // ymm4 = vpcmp result
                                            // ymm5 = output

    // ".rob1:\t\n"
    "vpermd ymm1, ymm2, ymm1\t\n"           // permute shifted_input, inplace
    "vpcmpgtd ymm4, ymm0, ymm1\t\n"         // Compare orig_input and shift_input, store in ymm4
    "vpsubd ymm3, ymm3, ymm4\t\n"           // Inc accumulate if cmp set bits

    "vpermd ymm1, ymm2, ymm1\t\n"           // permute shifted_input, inplace
    "vpcmpgtd ymm4, ymm0, ymm1\t\n"         // Compare orig_input and shift_input, store in ymm4
    "vpsubd ymm3, ymm3, ymm4\t\n"           // Inc accumulate if cmp set bits

    "vpermd ymm1, ymm2, ymm1\t\n"           // permute shifted_input, inplace
    "vpcmpgtd ymm4, ymm0, ymm1\t\n"         // Compare orig_input and shift_input, store in ymm4
    "vpsubd ymm3, ymm3, ymm4\t\n"           // Inc accumulate if cmp set bits

    "vpermd ymm1, ymm2, ymm1\t\n"           // permute shifted_input, inplace
    "vpcmpgtd ymm4, ymm0, ymm1\t\n"         // Compare orig_input and shift_input, store in ymm4
    "vpsubd ymm3, ymm3, ymm4\t\n"           // Inc accumulate if cmp set bits

    "vpermd ymm1, ymm2, ymm1\t\n"           // permute shifted_input, inplace
    "vpcmpgtd ymm4, ymm0, ymm1\t\n"         // Compare orig_input and shift_input, store in ymm4
    "vpsubd ymm3, ymm3, ymm4\t\n"           // Inc accumulate if cmp set bits

    "vpermd ymm1, ymm2, ymm1\t\n"           // permute shifted_input, inplace
    "vpcmpgtd ymm4, ymm0, ymm1\t\n"         // Compare orig_input and shift_input, store in ymm4
    "vpsubd ymm3, ymm3, ymm4\t\n"           // Inc accumulate if cmp set bits

    "vpermd ymm1, ymm2, ymm1\t\n"           // permute shifted_input, inplace
    "vpcmpgtd ymm4, ymm0, ymm1\t\n"         // Compare orig_input and shift_input, store in ymm4
    "vpsubd ymm3, ymm3, ymm4\t\n"           // Inc accumulate if cmp set bits


    // "dec r12\t\n"                          // Loop 7 times
    // "jnz .rob1\t\n"

    "vpermd  ymm5, ymm0, ymm3\t\n"          // Final permute to sort values

    "vmovdqu YMMWORD PTR [rcx], ymm5\t\n"   // Store sorted values


    :
    : [input_array] "r" (input_array),
      [permute_array] "r" (permute_array),
      [output_array] "r" (output_array)
    // : "memory"
    : "rax",
      "rbx",
      "rcx",
      "r12"
      // "memory"
  );
  return;
}

void insertionSort(int *arr, int *unused, int *unused2)
{
   int n = 8;
   int i, key, j;
   for (i = 1; i < n; i++)
   {
       key = arr[i];
       j = i-1;

       /* Move elements of arr[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
       while (j >= 0 && arr[j] > key)
       {
           arr[j+1] = arr[j];
           j = j-1;
       }
       arr[j+1] = key;
   }
}


int main(int argc, char *argv[]) {
  // Test Permute ASM instruction
  // check_permute();

  // Set up input and output arrays
  std::array<int, 8> custom_inputs = {7,6,5,4,3,2,1,0};
  int *inputs = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));
  for (int i = 0; i < 8; ++i) { inputs[i] = custom_inputs[i]; }

  int custom_permutes[8] = {1,2,3,4,5,6,7,0};
  int *permutes = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));
  for (int i = 0; i < 8; ++i) { permutes[i] = custom_permutes[i]; }

  int *outputs = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));

  // Make Timer
  Timer timer;

  // Test Intrinics
  std::cout << "Intrinsic test\n";
  time_sort(sort_ints_intrinsics, inputs, permutes, outputs);
  time_sort(sort_ints_asm, inputs, permutes, outputs);

  // Check correctness
  check_sort(inputs, outputs);

  // Test Insertion Sort
  std::cout << "Insertion Sort test\n";
  time_sort(insertionSort, inputs, permutes, outputs);




  return 0;
}
