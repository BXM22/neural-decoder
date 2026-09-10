# neural-decoder

A from-scratch C++ project: simulate a population of motor-cortex neurons
and decode their spikes into a velocity in real time. This is the core
loop behind an intracortical BCI (like Neuralink's Telepathy product) -
bin spikes, decode a command, hand it off - built here on synthetic data
so you can learn the signal-processing fundamentals without needing real
neural recordings.

The math (cosine tuning, Poisson spiking, population vector decoding) is
the textbook starting point from Georgopoulos et al.'s motor-cortex work.
Building it in C++ with real timing measurements shows both the
signal-processing fundamentals and the production instincts that matter
for a BCI decoding role: a real-time budget, no dynamic allocation in the
hot path, and metrics you can defend.

Step-by-step implementation notes live in [instructions.md](instructions.md).

## Requirements

- A C++17 compiler (`g++` on macOS/Linux)
- `make`

## Setup and build

```
make          # compile ./neural_decoder
make run      # compile if needed, then run
make clean    # remove the binary
```

`make run` prints population size, bin rate, decode latency, VAF, and
direction cosine similarity.

## Layout

```
src/types.hpp      Vec2 - 2D vector type
src/simulator.hpp  NeuronPopulation - cosine-tuned Poisson neurons
src/simulator.cpp  Population setup + sampleSpikeCounts
src/decoder.hpp    PopulationVectorDecoder::decode
src/main.cpp       Real-time loop + VAF / cosine similarity / latency
scripts/           Optional helpers (plots, etc.)
results/           Optional output (metrics, figures)
```

## Pipeline

```
true velocity → NeuronPopulation (spikes) → PopulationVectorDecoder → v_hat
```

Each neuron has a unit preferred direction. Its rate in a bin is
`max(0, baseline + gain * (PD · v))`; spike counts are Poisson with mean
`rate * dt`. The decoder is the reverse: a weighted sum of those preferred
directions, scaled by a closed-form calibration constant.

## Possible updates

The population vector decoder is the right first algorithm, not the last
one. These are natural next steps, ordered roughly from smallest change
to “this is a second project.” Drop comparison artifacts in `results/`.

- **Kalman filter decoder.** Model velocity as a smoothed AR(1) process
  and fit an observation matrix from simulated data (Wu et al. 2006).
  Compare VAF and latency against the population vector. This is the
  standard real-time BCI baseline; the population vector treats each bin
  independently, so it cannot use the fact that the hand does not jump.
- **Estimate preferred directions from data.** The current decoder is
  handed oracle PDs from the simulator. Fit them from `(spikes, velocity)`
  pairs instead — cosine-tuning regression per neuron — then decode with
  the fitted PDs. That is closer to what you do with a real array.
- **Center-out task.** Integrate decoded velocity into a cursor position
  and measure time-to-target on reaches to 8 targets. Instantaneous VAF
  is a decoder metric; time-to-target is the task metric BCI papers report.
- **No allocations in the hot path.** `sampleSpikeCounts` currently
  returns a new `std::vector<int>` every bin. Fill a preallocated buffer
  instead so decode latency is not mixed with heap traffic.
- **Sweep gain, bin size, and population size.** Plot VAF and cosine
  similarity vs. `N_NEURONS`, `GAIN`, and `BIN_MS`. That makes the
  signal-vs-Poisson-noise tradeoff visible instead of a single lucky run.
- **Dropped channels / nonstationarity.** Zero out a fraction of neurons
  mid-session, or slowly rotate PDs, and show how VAF falls. Real arrays
  lose units; a decoder that only works on a stationary oracle population
  is not yet a decoder you would ship.
- **Real recordings.** Public reaching-task datasets (e.g. Stanford NPTL)
  let you run the same decoder on real spikes instead of cosine-tuned
  Poisson neurons. Expect VAF to drop; that gap is the interesting result.
