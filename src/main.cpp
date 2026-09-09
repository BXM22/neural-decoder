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
    // Suggested constants (use these or pick your own):
    //   N_NEURONS   = 96
    //   BASELINE_HZ = 20.0
    //   GAIN        = 40.0     // Hz per unit speed at PD alignment
    //   BIN_MS      = 10.0     // 100 Hz bin rate, typical for BCI decoders
    //   N_BINS      = 20000    // ~200 seconds of simulated data

    // TODO(you): see README.md Step 4.
    //
    //   1. Construct a NeuronPopulation and a PopulationVectorDecoder
    //      (pass the population's preferred directions into the decoder).
    //
    //   2. Loop over N_BINS bins. Each iteration:
    //        - pick a true velocity (change reach direction every ~2 s)
    //        - sample spike counts from the population
    //        - decode them, timing the decode call in nanoseconds
    //        - update running sums for VAF and cosine similarity
    //
    //   3. Print population size, bin count, decode latency, VAF, and
    //      direction cosine similarity. There's a sample printf block
    //      in the README under "What done looks like".

    return 0;
}
