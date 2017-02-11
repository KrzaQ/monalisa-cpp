#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <stdint.h>
#include <time.h>

#include <algorithm>
#include <array>
#include <random>


constexpr int W = 256;
constexpr int H = 382;
constexpr int SZ = W * H;

const auto monalisa = []{
  std::array<uint8_t, SZ> ret;
  FILE *f = fopen("mona_small_gray.raw", "rb");
  fread(ret.data(), SZ, 1, f);
  fclose(f);
  return ret;
}();

constexpr size_t SPEC_CNT = 300;
constexpr size_t BEST_CNT = 2;

static thread_local std::mt19937 gen{0};
//thread_local std::mt19937 gen{std::random_device{}()};

template<typename T>
T random(T min, T max) {
  return std::uniform_int_distribution<T>{min, max}(gen);
}

struct specimen : std::array<uint8_t, SZ>
{
  using std::array<uint8_t, SZ>::array;
};

static std::array<specimen, SPEC_CNT> specimens;
static std::array<specimen, BEST_CNT> best_spec;
static std::array<size_t, BEST_CNT> best_indices;
static int step = 0;

void dump_best() {
  if (step % 10 != 0) {
    return;
  }

  printf("   Dumping step %i\r", step); fflush(stdout);
  for (size_t i = 0; i < 1; i++) {
    char fname[256] = {0};
    sprintf(fname, "out/best_%.6i_%.2lu.raw", step, i);
    FILE *f = fopen(fname, "wb");
    fwrite(specimens[best_indices[i]].data(), SZ, 1, f);
    fclose(f);
  }
}

void mutate() {
//  for (size_t i = 0; i < SPEC_CNT; i++) {
  for(specimen& s : specimens) {
    int x = random<int>(0, W - 2);
    int y = random<int>(0, H - 2);
    int w = random<int>(1, W - x - 1);
    int h = random<int>(1, H - y - 1);
    int c = random<uint8_t>(0, 255);

    for (int n = y; n < y + h; n++) {
      for (int m = x; m < x + w; m++) {
        s[n * W + m] =
          (s[n * W + m] + c) >> 1;
      }
    }
  }
}

double calc_score(specimen const& s) {
  double sc = 0.0;
  for (size_t j = 0; j < H; j++) {
    for (size_t i = 0; i < W; i++) {
      double a = s[j * W + i];
      double b = monalisa[j * W + i];
      sc += (a - b) * (a - b);
    }
  }
  return sc;
}

struct scores_st {
  double score;
  size_t idx;
};

void score() {
  static scores_st scores[SPEC_CNT];

  for (size_t i = 0; i < SPEC_CNT; i++) {
    scores[i].idx = i;
    scores[i].score = calc_score(specimens[i]);
  }

  std::sort(std::begin(scores), std::end(scores), [](auto const& l, auto const& r){
    return l.score < r.score;
  });

  for (size_t i = 0; i < BEST_CNT; i++) {
    best_indices[i] = scores[i].idx;
    //printf("%i %i %f\n", i, best[i], scores[i].score);
  }
}

void cross() {
  for (size_t i = 0; i < BEST_CNT; i++) {
    memcpy(best_spec[i].data(), specimens[best_indices[i]].data(), SZ);
  }

  for (size_t i = BEST_CNT; i < SPEC_CNT; i++) {
    memcpy(specimens[i].data(), best_spec[i % BEST_CNT].data(), SZ);
  }
}

int main(void) {
  // srand(time(NULL));



  for (;; step++) {
    //puts("Stage 1");
    mutate();

    //puts("Stage 2");
    score();

    // Dump best.
    //puts("Stage 3");
    cross();
    dump_best();    

    ///break;
  }

  return 0;
}

