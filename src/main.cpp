#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "decoder.hpp"
#include "simulator.hpp"
#include "types.hpp"



int main() {
    const int    N_NEURONS   = 96
    const double BASELINE_HZ = 20.0
    const double GAIN        = 40.0     // Hz per unit speed at PD alignment
    const double BIN_MS      = 10.0     // 100 Hz bin rate, typical for BCI decoders
    const int    N_BINS      = 20000    // ~200 seconds of simulated data

    NeuronPopulation pop(N_NEURONS,)






    return 0;
}
