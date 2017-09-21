#include "util.h"

extern void do_cache_flush();

////////////////////////////////////////////////
// PRINTING
////////////////////////////////////////////////
#ifndef DEBUG
void print_array(const int *array, std::string msg) {
  std::cout << msg + ": ";
  for (int i = 0; i < 8; ++i) {
    std::cout << array[i];
  }
  std::cout << "\n";
}

void print_ivec(__m256i vec, std::string msg) {
  // int buf[8];
  int *buf = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));

  _mm256_store_si256((__m256i *)buf, vec);
  std::cout << msg + ": ";
  for (int i = 0; i < 8; ++i) {
    std::cout << buf[i];
  }
  std::cout << "\n";
}
#endif


////////////////////////////////////////////////
// TIMING
////////////////////////////////////////////////

constexpr int WARMUP = 10000;
constexpr int REPEATS = 50000000;
typedef std::function<void(int *, int *, int *)> sort_func_t;
void __attribute__((optimize("O0"))) time_sort(sort_func_t sort_func, int *inputs, int *permutes, int *outputs) {
    int total_count = 0;
    auto start = std::chrono::system_clock::now();
    for (int i = -1 * WARMUP; i < REPEATS; ++i) {
      // do_cache_flus h();
      // std::cout << "itr\n";
      sort_func(inputs, permutes, outputs);
    }
      auto end = std::chrono::system_clock::now();
      // if (i >= 0)
        total_count += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "Avg Time taken = "
              << total_count / REPEATS
              << " nanoseconds with "
              << REPEATS << " repeats\n";
}

////////////////////////////////////////////////
// CHECKING
////////////////////////////////////////////////

void check_sort(const int *inputs, const int *outputs) {
  // Do a simple sort to check if sort was correct
  std::cout << "Checking Sort";
  std::cout << "\nInputs: \t";
  for (int i = 0; i < 8; ++i) {std::cout << inputs[i];}
  std::cout << "\nOutputs: \t";
  for (int i = 0; i < 8; ++i) {std::cout << outputs[i];}
  std::cout << "\nExpected: \t";
  std::array<int, 8> expected = {};
  for (int i = 0; i < 8; ++i) {expected[i] = inputs[i];}
  std::sort(expected.begin(), expected.end());
  for (int i = 0; i < 8; ++i) {std::cout << expected[i];}
  std::cout << "\n";
}

void check_permute() {
  int *inputs = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));
  for (int i = 0; i < 8; ++i) {
    inputs[i] = i;
  }
  print_array(inputs, "inputs");

  int *perm = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));
  int manual_perm[8] = {1,7,0,4,3,2,6,5};
  for (int i = 0; i < 8; ++i) {
    perm[i] = manual_perm[i];
  }
  print_array(perm, "perm");

  int *outputs = (int*)aligned_alloc(/*alignement*/64, /*size*/8*sizeof(int));

  __m256i i = _mm256_load_si256((__m256i *)inputs);
  __m256i p = _mm256_load_si256((__m256i *)perm);
  __m256i o = _mm256_permutevar8x32_epi32(i, p);
  _mm256_store_si256((__m256i *)outputs, o);
  print_array(outputs, "outputs");

  for (int i = 0; i < 8; ++i) {
    assert(perm[i] == outputs[i]);
  }
  return;
}
