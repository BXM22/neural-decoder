#pragma once
#include <vector>
#include "types.hpp"


class PopulationVectorDecoder {
public:
    explicit PopulationVectorDecoder(std::vector<Vec2> preferred_directions, double gain, double dt_s)
        : pd_(std::move(preferred_directions)),
          C_(2.0/(pd_.size() * gain *dt_s)) {}

   
    Vec2 decode(const std::vector<int>& spike_counts) const {
        Vec2 sum{0.0,0.0};
        for(size_t i = 0; i < pd_.size(); i++){
            sum += pd_[i]*static_cast<double>(spike_counts[i]);
        }
        return sum * C_;
    }

private:
    std::vector<Vec2> pd_;
    double C_;
};
