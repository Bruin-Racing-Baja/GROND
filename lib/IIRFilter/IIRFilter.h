#ifndef IIR_FILTER_H
#define IIR_FILTER_H
#include <stdint.h>

class IIRFilter {
 public:
  IIRFilter(const float b_[], const float a_[], uint32_t M_, uint32_t N_);
  ~IIRFilter();

  float update(float new_x);

  float get();

 private:
  float* b;
  float* a;
  float* y;
  float* x;
  uint32_t M;
  uint32_t N;
};
#define iirfilter_h
#endif