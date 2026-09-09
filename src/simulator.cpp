#include "simulator.hpp"
#include <cmath>
#include <algorithm>

NeuronPopulation::NeuronPopulation(int n_neurons, double baseline_hz,
                                    double gain_hz_per_unit_speed, unsigned seed)
    : rng_(seed) {
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * M_PI);

    pd_.reserve(n_neurons);
    baseline_.reserve(n_neurons);
    for (int i = 0; i < n_neurons; ++i) {
        double theta = angle_dist(rng_);
        pd_.emplace_back(std::cos(theta) * gain_hz_per_unit_speed,
                          std::sin(theta) * gain_hz_per_unit_speed);
        baseline_.push_back(baseline_hz);
    }
}

std::vector<int> NeuronPopulation::sampleSpikeCounts(const Vec2& velocity, double dt_s) {
    // TODO(you): for each neuron i, do three things - see README.md Step 1
    // for the full walkthrough and the math behind each line:
    //
    //   1. Compute its instantaneous firing rate:
    //        rate_i = baseline_[i] + pd_[i].dot(velocity)
    //      (this is the cosine tuning model - dot product of the neuron's
    //      preferred-direction vector with the true velocity)
    //
    //   2. Clip it at zero - firing rates can't go negative:
    //        rate_i = std::max(0.0, rate_i)
    //
    //   3. Draw a spike count from a Poisson distribution with mean
    //      rate_i * dt_s (expected number of spikes in this time bin),
    //      using std::poisson_distribution<int> and rng_.
    //
    // Return a vector<int> of length pd_.size(), one spike count per neuron.

    std::vector<int> counts(pd_.size(), 0);
    return counts;
}
