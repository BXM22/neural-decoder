#include <vector>
#include <random>
#include <algorithm>

struct Vec2{
     double x,y;
     double dot(const Vec2& other) const{
        return x * other.x +y * other.y;
     }
};

class NeuronPopulation {
    public:
        NeuronPopulation(std::vector<Vec2> preferredDirections,
                        std::vector<double> baselines,unsigned int seed)
                        :PD_(std::move(preferredDirections)),
                         baseline_(std::move(baselines)),
                         rng_(seed)
                         {}
    // v: true velocity right now
    // dt_s: bin duration in seconds
        std::vector<int> sampleSpikeCounts( const Vec2& v, double dt_s){
            std::vector<int> counts;
            counts.reserve(PD_size());

            //loop thriugh the neuron population
            for(size_t i = 0: i < PD_.size(); i++){
                //cosine tuning
                double raw = baseline_[i] + PD_[i].dot(v);
                //can't be negative so the rate has to be above zero
                double rate = std::max(0,raw);
                //conversion
                double lambda = rate * dt_s;

                //poisson smapling
                std::poisson_distribution<int> poissonDist(lambda);
                int k = poissonDist(rng_);
                
                counts.pushback(k);

            }
        }
    private:
        std::vector<Vec2> PD_;
        std::vector<double
}