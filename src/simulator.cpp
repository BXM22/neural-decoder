#include "simulator.hpp"
#include <cmath>
#include <algorithm>

NeuronPopulation::NeuronPopulation(int n_neurons, double baseline_hz,
                                    double gain_hz_per_unit_speed, unsigned seed)
    : gain_(gain_hz_per_unit_speed), rng_(seed) {
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * M_PI);

    pd_.reserve(n_neurons);
    baseline_.reserve(n_neurons);

    for (int i = 0; i < n_neurons; i++) {
        double theta = angle_dist(rng_);
        pd_.emplace_back(std::cos(theta), std::sin(theta));
        baseline_.push_back(baseline_hz);
    }
}

std::vector<int> NeuronPopulation::sampleSpikeCounts(const Vec2& velocity, double dt_s) {
    std::vector<int> counts(pd_.size(), 0);
    for (int i = 0; i < static_cast<int>(pd_.size()); i++) {
        double raw = baseline_[i] + gain_ * pd_[i].dot(velocity);
        double rate = std::max(0.0, raw);
        double lambda = rate * dt_s;

        std::poisson_distribution<int> poissonDist(lambda);
        counts[i] = poissonDist(rng_);
    }
    return counts;
}
