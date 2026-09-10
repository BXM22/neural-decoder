#pragma once
#include <vector>
#include <random>
#include "types.hpp"

// Simulates a population of motor-cortex-like neurons whose instantaneous
// firing rate follows classic cosine ("Georgopoulos") velocity tuning:
//
//   rate_i(t) = max(0, b0_i + gain_ * (PD_i . v(t)))
//
// where PD_i is neuron i's unit preferred-direction vector and b0_i is
// its baseline rate. Spikes within each bin are drawn from a Poisson
// process.
class NeuronPopulation {
public:
    NeuronPopulation(int n_neurons, double baseline_hz, double gain_hz_per_unit_speed,
                      unsigned seed);

    const std::vector<Vec2>& preferredDirections() const { return pd_; }
    const std::vector<double>& baselines() const { return baseline_; }
    double gain() const { return gain_; }
    int size() const { return static_cast<int>(pd_.size()); }

   
    std::vector<int> sampleSpikeCounts(const Vec2& velocity, double dt_s);

private:
    std::vector<Vec2> pd_;         // unit preferred direction, one per neuron
    std::vector<double> baseline_; // baseline firing rate (Hz), one per neuron
    double gain_;                  // Hz per unit speed at PD alignment
    std::mt19937 rng_;
};
