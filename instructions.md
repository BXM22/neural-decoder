# Implementation instructions

Work through these steps in order. Each one only needs what came before
it. For project background and how to build, see [README.md](README.md).

The program currently compiles with Steps 3 and 4 stubbed out. Use a
successful `make` as your checkpoint after each remaining step.

---

## Step 0: `types.hpp` - a 2D vector

You need addition, subtraction, scalar multiply, `+=`, Euclidean `norm()`,
and a `dot()` product. These are the only operations the rest of the
project uses.

```
(a + b).x = a.x + b.x          (same for y)
(a * s).x = a.x * s
a.dot(b)  = a.x * b.x + a.y * b.y
a.norm()  = sqrt(a.x^2 + a.y^2)
```

Fill in the function bodies already declared on `Vec2` in `types.hpp`.

---

## Step 1: `simulator.cpp` - set up the neuron population

Each neuron `i` has a "preferred direction" `PD_i` - a **unit vector**
encoding the direction it responds to. Tuning strength (gain) is kept as
a single, separate scalar shared by the population (`gain_hz_per_unit_speed`),
not folded into `PD_i` - this matters for Step 3, see the note there.

In the `NeuronPopulation` constructor, loop over `n_neurons` and for each
one:

1. Draw an angle `θ` uniformly from `[0, 2π)` using `rng_`.
2. Store `PD_i = (cos θ, sin θ)` in `pd_` - a **unit vector**, magnitude 1.
3. Store `baseline_hz` in `baseline_`.

Separately, store the constructor's `gain_hz_per_unit_speed` argument in
a `gain_` member (one scalar for the whole population) - Step 2 will
multiply by it explicitly instead of it being pre-baked into `PD_i`.

`rng_` is already seeded from the constructor argument. Use
`std::uniform_real_distribution<double>` for the angles. `reserve` on the
vectors first if you want to avoid reallocations.

---

## Step 2: `simulator.cpp` - generate spikes from a velocity

**Concept: cosine tuning.** A neuron's instantaneous firing rate given the
true velocity `v(t)` is:

```
rate_i(t) = max(0, baseline_i + gain_ * (PD_i . v(t)))
```

The dot product is the key idea: when velocity points in the neuron's
preferred direction, the dot product is large and positive, so the neuron
fires a lot above baseline. When velocity points opposite, the dot product
is negative, and the rate clips at zero (firing rates can't go negative).
`gain_` sets how many Hz above baseline a neuron gains per unit of aligned
speed. This one line of math is essentially the entire encoding model used
in real motor-cortex BCI research.

**Concept: Poisson spiking.** Real spike counts in a short time bin are
well-modeled as a Poisson random variable with mean `rate * dt`. Given a
rate in Hz and a bin duration `dt_s` in seconds, the expected spike count
in that bin is `rate * dt_s`. Use `std::poisson_distribution<int>` seeded
from the class's `rng_` member to draw an actual (noisy) count.

**Your task:** in `sampleSpikeCounts`, loop over all neurons and, for each
one, compute its rate (`baseline_i + gain_ * PD_i.dot(v)`), clip it at
zero, then draw a Poisson sample for the spike count. Return the vector of
counts.

Sanity check once done: print `sampleSpikeCounts` for a velocity aligned
with a neuron's PD vs. opposite to it - the aligned neuron should fire much
more.

---

## Step 3: `decoder.hpp` - decode spikes back into a velocity

**Concept: population vector algorithm.** This is the reverse of Step 2.
If a neuron fired a lot, it's "voting" for its preferred direction. Add up
every neuron's vote, weighted by how much it fired, then rescale by a
calibration constant that undoes the encoding-side gain and bin duration:

```
v_hat(t) = C * sum_i spikes_i(t) * PD_i,     where C = 2 / (n_neurons * gain_ * dt_s)
```

**Why `C` looks like that (and why it isn't just `1/n_neurons`):** `PD_i`
here are *unit* vectors uniformly spread around the circle. Two facts fall
out of that: (1) `sum_i PD_i ≈ 0` for a reasonably large population, so a
neuron's baseline firing washes out of the vote sum on its own - no
explicit baseline subtraction needed. (2) `sum_i (PD_i · v) * PD_i ≈
(n_neurons / 2) * v` (a standard identity for vectors spread uniformly
over a circle). Combined with `E[spikes_i] = rate_i * dt_s`, that means
`sum_i spikes_i * PD_i ≈ (n_neurons * gain_ * dt_s / 2) * v` in
expectation - so you have to divide by exactly that factor to get `v`
back out, not just by `n_neurons`. Skipping the `gain_` and `dt_s` terms
in the calibration constant is the single easiest way to end up with a
"correctly implemented" decoder whose direction is right (good cosine
similarity) but whose magnitude is off by many times (terrible VAF,
since VAF - unlike cosine similarity - is scale-sensitive).

Because `C` depends on `gain_` and `dt_s`, `PopulationVectorDecoder` needs
both at construction time, in addition to the preferred directions - see
the updated constructor call in Step 4.

**Your task:** in `PopulationVectorDecoder::decode`, sum `spike_counts[i] *
pd_[i]` over all neurons, multiply by `C` (computed once in the
constructor and cached as a member), and return the result as a `Vec2`.

This is deliberately the simplest decoder that could work - no matrix
inversion, no training step, just a weighted sum with one closed-form
calibration constant - which is why real BCI systems still use a version
of it as a real-time baseline. (A natural extension once this works:
replace it with a Kalman filter, which uses the same preferred-direction
structure but models velocity as a smoothed, correlated-over-time process
instead of decoding each bin independently.)

**Sanity check before wiring up `main.cpp`:** hand-build a `spike_counts`
vector where only the neurons whose `PD_i` sits near one direction have
nonzero counts, and confirm `decode` returns a `Vec2` pointing close to
that direction (and, if you know the population size and gain, of roughly
the right magnitude too - use this to catch calibration-constant bugs
before they hide inside end-to-end VAF numbers).

---

## Step 4: `main.cpp` - the real-time loop, quality, and latency

Suggested constants (also listed as comments in `main.cpp`):

```
N_NEURONS   = 96
BASELINE_HZ = 20.0
GAIN        = 40.0      // Hz per unit speed at PD alignment
BIN_MS      = 10.0      // 100 Hz bin rate
N_BINS      = 20000     // ~200 seconds of simulated data
SPEED       = 1.0       // magnitude of the simulated velocity, unitless
```

`SPEED` matters: it's the `|v(t)|` you'll actually simulate, and because
firing rates clip at zero, the encoder isn't perfectly linear in `v` - the
calibration constant `C` derived in Step 3 is an approximation that gets
better the less clipping happens (i.e., when `gain_ * SPEED` isn't wildly
larger than `baseline_hz`). `SPEED = 1.0` with the constants above keeps
you in a reasonable regime; if you change `GAIN` or `BASELINE_HZ`, expect
to revisit `SPEED` too, and don't be surprised if VAF is a bit more
sensitive to these knobs than cosine similarity is.

Construct a `NeuronPopulation` and a `PopulationVectorDecoder` (pass
`pop.preferredDirections()`, `GAIN`, and `dt_s` into the decoder's
constructor - it needs all three to compute `C`). Then loop `N_BINS`
times:

1. Change the target reach direction every ~2 s (`t % 200 == 0` at 10 ms
   bins) by sampling a new angle uniformly on `[0, 2π)`.
2. Build the true velocity as `(cos θ, sin θ) * SPEED`.
3. `auto spikes = pop.sampleSpikeCounts(true_v, dt_s);`
4. Time `decoder.decode(spikes)` with `std::chrono::high_resolution_clock`.
5. Accumulate running sums for the two accuracy metrics below.

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

Print population size, bin count, decode latency (avg and max, in
microseconds), VAF, and cosine similarity when the loop finishes.

---

## What "done" looks like

With all steps implemented, `make run` should print something like:

```
Neural population : 96 neurons, 20 Hz baseline, cosine velocity tuning
Bins decoded      : 20000 (100.0 Hz bin rate, 200 s simulated)
Decode latency    : avg 0.0X us / bin, max X.XX us / bin
Decode accuracy   : VAF = 0.7-0.9ish, direction cosine similarity = 0.9+
```

If cosine similarity looks great but VAF is far below that (even strongly
negative), that's the signature of a calibration-constant bug, not a
direction bug - re-check `C` in Step 3 before touching anything else. If
both metrics are weak, it's more likely the `GAIN` constant in
`main.cpp` is too low relative to `BASELINE_HZ` (weak tuning drowned out
by Poisson noise) - try raising it and see how the tradeoff between
signal and noise plays out. That tradeoff is itself worth a sentence in
your portfolio writeup.

## Stretch goals (after Step 4 works)

- **Kalman filter decoder.** Model velocity as a smoothed AR(1) process
  and fit an observation matrix from simulated data via least squares -
  the standard approach in real BCI decoding (Wu et al. 2006). Compare its
  VAF and latency against the population vector decoder. Drop artifacts in
  `results/`.
- **Center-out task.** Instead of just measuring instantaneous velocity
  error, integrate decoded velocity into a cursor position and measure
  time-to-target across discrete reaches to 8 targets - the same paradigm
  real BCI papers report results in.
- **Swap in real data.** Public datasets like the Stanford Neural
  Prosthetics Translational Lab's released reaching-task recordings let
  you re-run this same decoder on real spikes instead of simulated ones.
