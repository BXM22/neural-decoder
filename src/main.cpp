#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "decoder.hpp"
#include "simulator.hpp"
#include "types.hpp"

// Real-time decoding benchmark loop:
//   1. Simulate a population of cosine-tuned neurons (simulator.hpp/cpp)
//   2. Feed their spikes, bin by bin, into a Population Vector decoder
//   3. Time each decode call and compare decoded vs. true velocity
//
// This mimics the core loop of an intracortical BCI: bin spikes -> decode
// a velocity command -> hand it to whatever the command drives (cursor,
// robotic arm, etc.), all under a real-time latency budget.

int main() {
    constexpr int N_NEURONS = 96;
    constexpr double BASELINE_HZ = 20.0;
    constexpr double GAIN = 40.0;       // Hz per unit speed at PD alignment
    constexpr double BIN_MS = 10.0;     // 100 Hz bin rate, typical for BCI decoders
    constexpr int N_BINS = 20000;       // ~200 seconds of simulated data

    NeuronPopulation pop(N_NEURONS, BASELINE_HZ, GAIN, /*seed=*/42);
    PopulationVectorDecoder decoder(pop.preferredDirections());

    std::mt19937 rng(7);
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * M_PI);

    double dt_s = BIN_MS / 1000.0;
    double true_speed = 1.0;
    double target_angle = angle_dist(rng);

    // TODO(you): accumulate whatever running sums you need for the
    // accuracy metrics below - see README.md Step 3. You'll want, at
    // minimum: sum of squared decode error, sum of squared true velocity
    // (for VAF), and sum(true . decoded), sum(|true|^2), sum(|decoded|^2)
    // (for cosine similarity between true and decoded direction).

    long long total_latency_ns = 0;
    long long max_latency_ns = 0;

    for (int t = 0; t < N_BINS; ++t) {
        // Change reach direction every ~2s, like successive center-out trials.
        if (t % 200 == 0) target_angle = angle_dist(rng);
        Vec2 true_v(std::cos(target_angle) * true_speed, std::sin(target_angle) * true_speed);

        auto spikes = pop.sampleSpikeCounts(true_v, dt_s);

        auto t0 = std::chrono::high_resolution_clock::now();
        Vec2 decoded_v = decoder.decode(spikes);
        auto t1 = std::chrono::high_resolution_clock::now();

        long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        total_latency_ns += ns;
        if (ns > max_latency_ns) max_latency_ns = ns;

        // TODO(you): update your accuracy accumulators here using true_v
        // and decoded_v.
    }

    double avg_latency_us = (total_latency_ns / 1000.0) / N_BINS;
    double max_latency_us = max_latency_ns / 1000.0;

    // TODO(you): compute final metrics from your accumulators:
    //   double vaf = 1.0 - (sum_sq_err / sum_sq_true);     // 1.0 = perfect
    //   double cosine_sim = sum_true_dot_hat / std::sqrt(sum_true_norm * sum_hat_norm);

    std::printf("Neural population : %d neurons, %.0f Hz baseline, cosine velocity tuning\n",
                N_NEURONS, BASELINE_HZ);
    std::printf("Bins decoded      : %d (%.1f Hz bin rate, %.0f s simulated)\n",
                N_BINS, 1000.0 / BIN_MS, N_BINS * dt_s);
    std::printf("Decode latency    : avg %.2f us / bin, max %.2f us / bin\n",
                avg_latency_us, max_latency_us);
    // std::printf("Decode accuracy   : VAF = %.3f, direction cosine similarity = %.3f\n",
    //             vaf, cosine_sim);
    return 0;
}
