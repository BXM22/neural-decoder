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

The program compiles with remaining TODOs stubbed out. Until you finish
Steps 3 and 4 it exits immediately with no metrics.

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
