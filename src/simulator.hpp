#pragma once
#include <vector>
#include <random>
#include "types.hpp"

// Simulates a population of motor-cortex-like neurons whose instantaneous
// firing rate follows classic cosine ("Georgopoulos") velocity tuning:
//
//   rate_i(t) = max(0, b0_i + PD_i . v(t))
//
// where PD_i is neuron i's preferred-direction vector (its gain and
// direction of maximal modulation) and b0_i is its baseline rate.
// Spikes within each bin are drawn from a Poisson process - see README.md
// "Step 1" for the derivation and what to implement below.
class NeuronPopulation {
public:
    NeuronPopulation(int n_neurons, double baseline_hz, double gain_hz_per_unit_speed,
                      unsigned seed);

    const std::vector<Vec2>& preferredDirections() const { return pd_; }
    const std::vector<double>& baselines() const { return baseline_; }
    int size() const { return static_cast<int>(pd_.size()); }

    // Draw a spike count per neuron for one time bin of length dt_s given
    // the true hand/cursor velocity during that bin.
    // TODO(you): implement in simulator.cpp - see README.md Step 1.
    std::vector<int> sampleSpikeCounts(const Vec2& velocity, double dt_s);

private:
    std::vector<Vec2> pd_;         // preferred direction * gain, one per neuron
    std::vector<double> baseline_; // baseline firing rate (Hz), one per neuron
    std::mt19937 rng_;
};
