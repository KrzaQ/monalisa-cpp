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

thread_local std::mt19937 gen{0};
//thread_local std::mt19937 gen{std::random_device{}()};

template<typename T>
T random(T min, T max) {
  return std::uniform_int_distribution<T>{min, max}(gen);
}

uint8_t specimen[SPEC_CNT][SZ];
size_t best_spec[BEST_CNT][SZ];
size_t best[BEST_CNT];
int step = 0;

void dump_best() {
  if (step % 10 != 0) {
    return;
  }

  printf("   Dumping step %i\r", step); fflush(stdout);
  for (size_t i = 0; i < 1; i++) {
    char fname[256] = {0};
    sprintf(fname, "out/best_%.6i_%.2i.raw", step, i);
    FILE *f = fopen(fname, "wb");
    fwrite(specimen[best[i]], SZ, 1, f);
    fclose(f);
  }
}

void mutate() {
  for (size_t i = 0; i < SPEC_CNT; i++) {
    int x = random<int>(0, W - 2);
    int y = random<int>(0, H - 2);
    int w = random<int>(1, W - x - 1);
    int h = random<int>(1, H - y - 1);
    int c = random<uint8_t>(0, 255);

    for (int n = y; n < y + h; n++) {
      for (int m = x; m < x + w; m++) {
        specimen[i][n * W + m] =
          (specimen[i][n * W + m] + c) >> 1;
      }
    }
  }
}

double score_me(uint8_t *sp) {
  double sc = 0.0;
  for (size_t j = 0; j < H; j++) {
    for (size_t i = 0; i < W; i++) {
      double a = sp[j * W + i];
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
    scores[i].score = score_me(specimen[i]);
  }

  std::sort(std::begin(scores), std::end(scores), [](auto const& l, auto const& r){
    return l.score < r.score;
  });

  for (size_t i = 0; i < BEST_CNT; i++) {
    best[i] = scores[i].idx;
    //printf("%i %i %f\n", i, best[i], scores[i].score);
  }
}

void cross() {
  for (size_t i = 0; i < BEST_CNT; i++) {
    memcpy(best_spec[i], specimen[best[i]], SZ);
  }

  for (size_t i = BEST_CNT; i < SPEC_CNT; i++) {
    memcpy(specimen[i], best_spec[i % BEST_CNT], SZ);
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

