#include <vector>

class AlphaBetaFilter {
 private:
  double state_;
  double derivative_;
  double alpha_;
  double beta_;

 public:
  AlphaBetaFilter(double alpha, double beta)
      : state_(0.0), derivative_(0.0), alpha_(alpha), beta_(beta) {}

  double filter(double measurement, double dt) {
    double predicted_state = state_ + dt * derivative_;
    double residual = measurement - predicted_state;

    state_ = predicted_state + alpha_ * residual;
    derivative_ = derivative_ + beta_ * residual / dt;

    return state_;
  }
};

class MovingAverageFilter {
 private:
  std::vector<double> buffer_;
  std::size_t buffer_size_;

 public:
  MovingAverageFilter(std::size_t buffer_size) : buffer_size_(buffer_size) {}

  double filter(double measurement) {
    buffer_.push_back(measurement);

    if (buffer_.size() > buffer_size_) {
      buffer_.erase(buffer_.begin());
    }

    double sum = 0.0;
    for (double value : buffer_) {
      sum += value;
    }

    return sum / buffer_.size();
  }
};

class UltraLowLatencyFilter {
 private:
  AlphaBetaFilter alpha_beta_filter_;
  MovingAverageFilter moving_average_filter_;

 public:
  UltraLowLatencyFilter(double alpha, double beta, std::size_t buffer_size)
      : alpha_beta_filter_(alpha, beta), moving_average_filter_(buffer_size) {}

  double filter(double measurement, double dt) {
    double alpha_beta_output = alpha_beta_filter_.filter(measurement, dt);
    return moving_average_filter_.filter(alpha_beta_output);
  }
};