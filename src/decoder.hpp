#pragma once
#include <vector>
#include "types.hpp"

// Population Vector Algorithm (Georgopoulos et al.) - the classic, simplest
// possible real-time decoder: velocity estimate is a weighted sum of each
// neuron's preferred direction, weighted by how much it fired this bin.
//
//   v_hat(t) = (1 / n_neurons) * sum_i spikes_i(t) * PD_i
//
// O(n_neurons) per bin, no matrix math, no training beyond knowing each
// neuron's preferred direction - which is why it's still used as a
// real-time baseline in real BCI systems. See README.md Step 2.
class PopulationVectorDecoder {
public:
    explicit PopulationVectorDecoder(std::vector<Vec2> preferred_directions)
        : pd_(std::move(preferred_directions)) {}

    // TODO(you): implement below - see README.md Step 2.
    Vec2 decode(const std::vector<int>& spike_counts) const {
        // Sum spike_counts[i] * pd_[i] over all neurons, then divide by
        // pd_.size() so the estimate's scale doesn't depend on population
        // size.
        return Vec2{0.0, 0.0};
    }

private:
    std::vector<Vec2> pd_;
};
