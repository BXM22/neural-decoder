# neural-decoder

A bare-bones, from-scratch C++ project: simulate a population of motor-cortex
neurons and decode their spikes into a velocity in real time. This is the
core loop behind an intracortical BCI (like Neuralink's Telepathy product) -
bin spikes, decode a command, hand it off - built here on synthetic data so
you can learn the signal-processing fundamentals without needing real
neural recordings.

The project is a **scaffold, not a finished program.** The boilerplate
(project structure, RNG setup, timing harness) is filled in; the three
pieces that actually matter are left as `TODO(you)` blocks for you to
implement. Do them in order - each one only needs what came before it.

## Why this project

The math here (cosine tuning, Poisson spiking, population vector decoding)
is the textbook starting point for neural decoding, described in
Georgopoulos et al.'s classic motor cortex work. It's exactly the kind of
"simple solution grounded in first principles" a BCI decoding role is
looking for - and building it yourself, in C++, with real timing
measurements, shows both the signal-processing fundamentals and the
production-engineering instincts (real-time budget, no dynamic allocation
in the hot path, etc.) that matter for the job.

## How the pieces fit together

```
src/types.hpp      Vec2 - a tiny 2D vector type. Done, no TODOs.
src/simulator.hpp  NeuronPopulation - declares the neuron model.
src/simulator.cpp  NeuronPopulation::sampleSpikeCounts - TODO, Step 1.
src/decoder.hpp    PopulationVectorDecoder::decode - TODO, Step 2.
src/main.cpp       Real-time loop + accuracy metrics - TODO, Step 3.
```

Build with `make`, run with `make run` (or `./neural_decoder` after
building). The program currently compiles and runs with all TODOs stubbed
out - it just reports zero/garbage accuracy until you fill them in. Use
that as your checkpoint after each step.

---

## Step 1: `simulator.cpp` - generate spikes from a velocity

**Concept: cosine tuning.** Each neuron `i` has a "preferred direction"
`PD_i` - a 2D vector already set up for you in the constructor, encoding
both a direction and a gain (how strongly the neuron responds). A neuron's
instantaneous firing rate given the true velocity `v(t)` is:

```
rate_i(t) = max(0, baseline_i + PD_i . v(t))
```

The dot product is the key idea: when velocity points in the neuron's
preferred direction, the dot product is large and positive, so the neuron
fires a lot above baseline. When velocity points opposite, the dot product
is negative, and the rate clips at zero (firing rates can't go negative).
This one line of math is essentially the entire encoding model used in
real motor-cortex BCI research.

**Concept: Poisson spiking.** Real spike counts in a short time bin are
well-modeled as a Poisson random variable with mean `rate * dt`. Given a
rate in Hz and a bin duration `dt_s` in seconds, the expected spike count
in that bin is `rate * dt_s`. Use `std::poisson_distribution<int>` seeded
from the class's `rng_` member to draw an actual (noisy) count.

**Your task:** in `sampleSpikeCounts`, loop over all neurons and, for each
one, compute its rate, clip it at zero, then draw a Poisson sample for the
spike count. Return the vector of counts.

Sanity check once done: print `sampleSpikeCounts` for a velocity aligned
with a neuron's PD vs. opposite to it - the aligned neuron should fire much
more.

## Step 2: `decoder.hpp` - decode spikes back into a velocity

**Concept: population vector algorithm.** This is the reverse of Step 1.
If a neuron fired a lot, it's "voting" for its preferred direction. Add up
every neuron's vote, weighted by how much it fired:

```
v_hat(t) = (1 / n_neurons) * sum_i spikes_i(t) * PD_i
```

**Your task:** in `PopulationVectorDecoder::decode`, sum `spike_counts[i] *
pd_[i]` over all neurons, divide by the neuron count, and return the
result as a `Vec2`.

This is deliberately the simplest decoder that could work - no matrix
inversion, no training step, just a weighted sum, which is why real BCI
systems still use it as a real-time baseline. (A natural extension once
this works: replace it with a Kalman filter, which uses the same
preferred-direction structure but models velocity as a smoothed,
correlated-over-time process instead of decoding each bin independently.)

## Step 3: `main.cpp` - measure decode quality and latency

The loop already generates a changing target direction, calls your
simulator and decoder, and times the decode call in nanoseconds. What's
missing is turning "true velocity vs. decoded velocity, 20,000 times" into
two summary numbers:

**VAF (variance accounted for)** - how much of the true velocity's
variance your decoder captures. 1.0 is a perfect decoder, 0.0 is no better
than always guessing zero:

```
VAF = 1 - (sum of squared error) / (sum of squared true velocity)
```

**Cosine similarity** - on average, how well-aligned is the decoded
direction with the true direction, ignoring magnitude:

```
cosine_similarity = sum(true . decoded) / sqrt(sum(|true|^2) * sum(|decoded|^2))
```

**Your task:** add running-sum accumulators before the loop, update them
each iteration using `true_v` and `decoded_v`, then compute and print both
metrics after the loop (there's a commented-out `printf` ready to
uncomment).

---

## What "done" looks like

With all three steps implemented, `make run` should print something like:

```
Neural population : 96 neurons, 20 Hz baseline, cosine velocity tuning
Bins decoded      : 20000 (100.0 Hz bin rate, 200 s simulated)
Decode latency    : avg 0.0X us / bin, max X.XX us / bin
Decode accuracy   : VAF = 0.7-0.9ish, direction cosine similarity = 0.9+
```

A VAF well below that with a correct implementation usually means the
`GAIN` constant in `main.cpp` is too low relative to `BASELINE_HZ` (weak
tuning drowned out by Poisson noise) - try raising it and see how the
tradeoff between signal and noise plays out. That tradeoff is itself worth
a sentence in your portfolio writeup.

## Stretch goals (after Step 3 works)

- **Kalman filter decoder.** Model velocity as a smoothed AR(1) process
  and fit an observation matrix from simulated data via least squares -
  the standard approach in real BCI decoding (Wu et al. 2006). Compare its
  VAF and latency against the population vector decoder.
- **Center-out task.** Instead of just measuring instantaneous velocity
  error, integrate decoded velocity into a cursor position and measure
  time-to-target across discrete reaches to 8 targets - the same paradigm
  real BCI papers report results in.
- **Swap in real data.** Public datasets like the Stanford Neural
  Prosthetics Translational Lab's released reaching-task recordings let
  you re-run this same decoder on real spikes instead of simulated ones.
