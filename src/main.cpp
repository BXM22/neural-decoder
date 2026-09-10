#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "decoder.hpp"
#include "simulator.hpp"
#include "types.hpp"



int main() {
    const int    N_NEURONS   = 96;
    const double BASELINE_HZ = 20.0;
    const double GAIN        = 40.0;    // Hz per unit speed at PD alignment
    const double BIN_MS      = 10.0;    // 100 Hz bin rate, typical for BCI decoders
    const int    N_BINS      = 20000;   // ~200 seconds of simulated data
    const double SPEED       = 1.0;
    const double dt_s        = BIN_MS / 1000.0;

    NeuronPopulation pop(N_NEURONS,BASELINE_HZ,GAIN,42u);
    PopulationVectorDecoder decoder(pop.preferredDirections(),GAIN,dt_s);

    std::mt19937 rng(1);
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * M_PI);

    double sse = 0.0;          // Σ |true - decoded|²     → VAF
    double ss_true = 0.0;      // Σ |true|²
    double sum_dot = 0.0;      // Σ true · decoded         → cosine
    double ss_dec = 0.0;       // Σ |decoded|²
    double lat_sum_ns = 0.0;
    double lat_max_ns = 0.0;

    double theta = 0.0;
    //one bin = one tick
    for (int t = 0; t < N_BINS; t++) {
        // 1. New reach direction every ~2 s. 200 bins × 10 ms = 2 s.
        if (t % 200 == 0) {
            theta = angle_dist(rng);
        }

        // 2. True velocity: unit vector in that direction, scaled by SPEED.
        Vec2 true_v{std::cos(theta) * SPEED, std::sin(theta) * SPEED};

        // 3. Encode: cosine-tuned Poisson spikes for this bin.
        auto spikes = pop.sampleSpikeCounts(true_v, dt_s);

        // 4. Decode, timed.
        auto t0 = std::chrono::high_resolution_clock::now();
        Vec2 v_hat = decoder.decode(spikes);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
        lat_sum_ns += ns;
        if (ns > lat_max_ns) lat_max_ns = ns;

        // 5. Update metric sums.
        Vec2 err = true_v - v_hat;
        sse     += err.dot(err);           // |err|²
        ss_true += true_v.dot(true_v);     // |true|²
        ss_dec  += v_hat.dot(v_hat);       // |decoded|²
        sum_dot += true_v.dot(v_hat);
    }
    const double vaf = 1.0 - sse / ss_true;
    const double cosine = sum_dot / std::sqrt(ss_true * ss_dec);
    const double avg_us = (lat_sum_ns / N_BINS) / 1000.0;
    const double max_us = lat_max_ns / 1000.0;
    const double bin_hz = 1000.0 / BIN_MS;
    const double sim_s = N_BINS * dt_s;
    std::printf("Neural population : %d neurons, %.0f Hz baseline, cosine velocity tuning\n",
                N_NEURONS, BASELINE_HZ);
    std::printf("Bins decoded      : %d (%.1f Hz bin rate, %.0f s simulated)\n",
                N_BINS, bin_hz, sim_s);
    std::printf("Decode latency    : avg %.2f us / bin, max %.2f us / bin\n",
                avg_us, max_us);
    std::printf("Decode accuracy   : VAF = %.3f, direction cosine similarity = %.3f\n",
                vaf, cosine);


    return 0;
}
