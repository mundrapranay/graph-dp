/**
 * @cite: https://github.com/google/differential-privacy/tree/main
 * Geomtric Distribution Adpated from Google DIfferential Privacy
 * 
*/
//
// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <cmath>
#include <cstdlib>
#include <memory>
#include <cstdint>
#include <limits>
#include <cfloat>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "openssl/rand.h"

// #define DBL_MANT_DIG 53

#if ABSL_HAVE_ATTRIBUTE(guarded_by)
#define ABSL_GUARDED_BY(x) __attribute__((guarded_by(x)))
#else
#define ABSL_GUARDED_BY(x)
#endif


namespace distributed_kcore {

// We usually expect DBL_MANT_DIG to be 53.
static_assert(DBL_MANT_DIG < 64,
              "Double mantissa must have less than 64 bits.");
static_assert(sizeof(double) == sizeof(uint64_t) &&
                  std::numeric_limits<double>::is_iec559 &&
                  std::numeric_limits<double>::radix == 2,
              "double representation is not IEEE 754 binary64.");

const constexpr int kMantDigits = DBL_MANT_DIG - 1;
const constexpr uint64_t kMantissaMask = (uint64_t{1} << kMantDigits) - 1ULL;
ABSL_CONST_INIT absl::Mutex global_mutex(absl::kConstInit);

class SecureURBG {
 public:
  using result_type = uint64_t;
  static SecureURBG& GetInstance() {
    static auto* kInstance = new SecureURBG;
    return *kInstance;
  }

  static constexpr result_type(min)() {
    return (std::numeric_limits<result_type>::min)();
  }
  static constexpr result_type(max)() {
    return (std::numeric_limits<result_type>::max)();
  }
  result_type operator()() {
    absl::WriterMutexLock lock(&mutex_);
    if (current_index_ + sizeof(result_type) > kBufferSize) {
        RefreshBuffer();
    }
    int old_index = current_index_;
    current_index_ += sizeof(result_type);
    result_type result;
    std::memcpy(&result, buffer_ + old_index, sizeof(result_type));
    return result;
  }

 private:
  SecureURBG() { buffer_ = new uint8_t[kBufferSize]; }
  ~SecureURBG() { delete[] buffer_; }
  // Refresh the cache with new random bytes.
  void RefreshBuffer() {
    int one_on_success = 0;
    {
        absl::MutexLock lock(&global_mutex);
        one_on_success = RAND_bytes(buffer_, kBufferSize);
    }
    if (one_on_success != 1) {
        std::cout << "Error during buffer refresh: OpenSSL's RAND_byte is expected to "
            "return 1 on success, but returned "
        << one_on_success << std::endl;
    }
    current_index_ = 0;
  }

  static constexpr int kBufferSize = 65536;
  // The current index in the cache.
  int current_index_ ABSL_GUARDED_BY(mutex_) = kBufferSize;
  uint8_t* buffer_ ABSL_GUARDED_BY(mutex_);
  absl::Mutex mutex_;
};

uint64_t Geometric() {
  uint64_t result = 1;
  uint64_t r = 0;
  while (r == 0 && result < 1023) {
    r = SecureURBG::GetInstance()();
    result += absl::countl_zero(r);
  }
  return result;
}

double UniformDouble() {
  uint64_t uint_64_number = SecureURBG::GetInstance()();
  // A random integer of Uniform[0, 2^kMantDigits).
  uint64_t i = uint_64_number & kMantissaMask;

  // Instead of throwing the leading 12 bits away, we use them to create
  // geometric random number.
  uint64_t j = uint_64_number >> kMantDigits;

  // exponent is the number of leading zeros in the first 11 bits plus one.
  uint64_t exponent = absl::countl_zero(j) - kMantDigits + 1;

  // Extra geometric sampling is needed only when the leading 11 bits are all 0.
  if (j == 0) {
    exponent += Geometric() - 1;
  }

  j = (uint64_t{1023} - exponent) << kMantDigits;
  if (ABSL_PREDICT_FALSE(exponent >= 1023)) {
    // Denormalized value. Extremely improbable.
    j = 0;
  }
  // Addition instead of bitwise or since the carry overflow increments the
  // floating point exponent, which is exactly what we want.
  i += j;
  double r;
  std::memcpy(&r, &i, sizeof(r));
  return r == 0 ? 1.0 : r;
}



static constexpr double kPi = 3.14159265358979323846;

// The square root of the maximum number n of Bernoulli trials from which a
// binomial sample is drawn. Larger values result in more fine grained noise,
// but increase the chance of sampling inaccuracies due to overflows. The
// probability of such an event will be roughly 2^-45 or less, if the square
// root is set to 2^57.
static constexpr double kBinomialBound = (double)(1LL << 57);


// Returns a sample drawn from the geometric distribution of probability
// p = 1 - e^-lambda, i.e. the number of bernoulli trial failures before the
// first success where the success probability is as defined above. lambda must
// be positive. If the result would be higher than the maximum int64_t, returns
// the maximum int64_t, which means that users should be careful around the edges
// of their distribution.
class GeometricDistribution {
 public:
    GeometricDistribution(double lambda) {
        lambda_ = lambda;
    }

    double GetUniformDouble() {
        return UniformDouble();
    }

    int64_t Sample() {
        if (lambda_ == std::numeric_limits<double>::infinity()) {
            return 0;
        }

        if (GetUniformDouble() >
            -1.0 * std::expm1(-1.0 * lambda_ * std::numeric_limits<int64_t>::max())) {
            return std::numeric_limits<int64_t>::max();
        }

        // Performs a binary search for the sample over the range of possible output
        // values. At each step we split the remaining range in two and pick the left
        // or right side proportional to the probability that the output falls within
        // that range, ending when we have only a single possible sample remaining.
        int64_t lo = 0;
        int64_t hi = std::numeric_limits<int64_t>::max();
        while (hi - lo > 1) {
            int64_t mid =
                lo - static_cast<int64_t>(std::floor(
                        (std::log(0.5) + std::log1p(std::exp(lambda_ * (lo - hi)))) /
                        lambda_));
            mid = std::min(std::max(mid, lo + 1), hi - 1);

            double q = std::expm1(lambda_ * (lo - mid)) / std::expm1(lambda_ * (lo - hi));
            if (GetUniformDouble() <= q) {
            hi = mid;
            } else {
            lo = mid;
            }
        }
        return hi - 1;
    }


 private:
  double lambda_;
};


} // namespace dsitributed_kcore
