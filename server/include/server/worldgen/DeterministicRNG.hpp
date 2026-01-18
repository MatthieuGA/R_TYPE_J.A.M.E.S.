/**
 * @file DeterministicRNG.hpp
 * @brief Deterministic Pseudo-Random Number Generator for world generation.
 *
 * This file provides a deterministic PRNG implementation based on the PCG
 * (Permuted Congruential Generator) algorithm. PCG is chosen for its:
 * - Excellent statistical properties
 * - Small state (128 bits)
 * - Fast execution
 * - Reproducibility across platforms
 *
 * The generator ensures that given the same seed, the same sequence of
 * numbers will be produced on any platform/compiler, which is critical
 * for deterministic world generation.
 */
#pragma once

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace worldgen {

/**
 * @brief PCG-based deterministic random number generator.
 *
 * This class provides a deterministic PRNG that produces identical sequences
 * for the same seed across all platforms. It's designed specifically for
 * world generation where reproducibility is essential.
 *
 * Example usage:
 * @code
 * DeterministicRNG rng(12345);  // Create with seed
 * uint32_t value = rng.Next();  // Get next random uint32
 * float normalized = rng.NextFloat();  // Get float in [0, 1)
 * int index = rng.NextInt(0, 10);  // Get int in [0, 10]
 *
 * // Reset to reproduce the same sequence
 * rng.SetSeed(12345);
 * @endcode
 */
class DeterministicRNG {
 public:
    /**
     * @brief Default constructor with seed 0.
     */
    DeterministicRNG() : state_(0), increment_(1) {
        SetSeed(0);
    }

    /**
     * @brief Constructs the RNG with a specific seed.
     *
     * @param seed The seed value for deterministic generation.
     */
    explicit DeterministicRNG(uint64_t seed) : state_(0), increment_(1) {
        SetSeed(seed);
    }

    /**
     * @brief Constructs the RNG with seed and stream (sequence) selection.
     *
     * The stream parameter allows multiple independent sequences from
     * the same seed, useful for different aspects of world generation.
     *
     * @param seed The seed value.
     * @param stream The stream/sequence selector.
     */
    DeterministicRNG(uint64_t seed, uint64_t stream)
        : state_(0), increment_((stream << 1u) | 1u) {
        SetSeed(seed);
    }

    /**
     * @brief Sets a new seed, resetting the generator state.
     *
     * After calling this with the same seed, the generator will produce
     * the identical sequence of numbers.
     *
     * @param seed The new seed value.
     */
    void SetSeed(uint64_t seed) {
        state_ = 0u;
        Next();  // Advance once to initialize
        state_ += seed;
        Next();  // Advance again to properly mix
    }

    /**
     * @brief Sets both seed and stream for the generator.
     *
     * @param seed The seed value.
     * @param stream The stream/sequence selector (must be odd, will be made
     * odd if even).
     */
    void SetSeedWithStream(uint64_t seed, uint64_t stream) {
        increment_ = (stream << 1u) | 1u;
        SetSeed(seed);
    }

    /**
     * @brief Generates the next random 32-bit unsigned integer.
     *
     * This is the core generation function. All other methods ultimately
     * use this to produce their results.
     *
     * @return A uniformly distributed random uint32_t.
     */
    uint32_t Next() {
        uint64_t old_state = state_;
        // PCG XSH-RR: Advance internal state
        state_ = old_state * kMultiplier + increment_;
        // Calculate output function (XSH-RR)
        uint32_t xorshifted =
            static_cast<uint32_t>(((old_state >> 18u) ^ old_state) >> 27u);
        uint32_t rot = static_cast<uint32_t>(old_state >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    /**
     * @brief Generates a random float in the range [0, 1).
     *
     * @return A uniformly distributed float in [0, 1).
     */
    float NextFloat() {
        // Use 24 bits for float precision (mantissa is 23 bits + implicit 1)
        return static_cast<float>(Next() >> 8) *
               (1.0f / static_cast<float>(1u << 24));
    }

    /**
     * @brief Generates a random double in the range [0, 1).
     *
     * @return A uniformly distributed double in [0, 1).
     */
    double NextDouble() {
        // Combine two 32-bit values for better double precision
        uint64_t a = Next();
        uint64_t b = Next();
        uint64_t combined = (a << 21) ^ b;
        return static_cast<double>(combined) *
               (1.0 / static_cast<double>(1ull << 53));
    }

    /**
     * @brief Generates a random integer in the range [min, max] (inclusive).
     *
     * Uses rejection sampling to ensure uniform distribution.
     *
     * @param min The minimum value (inclusive).
     * @param max The maximum value (inclusive).
     * @return A uniformly distributed integer in [min, max].
     */
    int NextInt(int min, int max) {
        if (min > max) {
            // Swap if reversed
            int temp = min;
            min = max;
            max = temp;
        }
        uint32_t range = static_cast<uint32_t>(max - min + 1);
        return min + static_cast<int>(NextBounded(range));
    }

    /**
     * @brief Generates a random integer in the range [0, bound) (exclusive).
     *
     * Uses Lemire's nearly divisionless method for unbiased bounded
     * generation.
     *
     * @param bound The exclusive upper bound (must be > 0).
     * @return A uniformly distributed integer in [0, bound).
     */
    uint32_t NextBounded(uint32_t bound) {
        if (bound == 0)
            return 0;

        // Lemire's method - nearly divisionless, unbiased
        uint64_t random = Next();
        uint64_t multiresult = random * static_cast<uint64_t>(bound);
        uint32_t leftover = static_cast<uint32_t>(multiresult);

        if (leftover < bound) {
            uint32_t threshold = (-bound) % bound;
            while (leftover < threshold) {
                random = Next();
                multiresult = random * static_cast<uint64_t>(bound);
                leftover = static_cast<uint32_t>(multiresult);
            }
        }
        return static_cast<uint32_t>(multiresult >> 32);
    }

    /**
     * @brief Generates a random float in the range [min, max].
     *
     * @param min The minimum value.
     * @param max The maximum value.
     * @return A uniformly distributed float in [min, max].
     */
    float NextFloatRange(float min, float max) {
        return min + NextFloat() * (max - min);
    }

    /**
     * @brief Selects a random index based on weighted probabilities.
     *
     * @param weights Vector of weights (must not be empty, weights > 0).
     * @return The selected index [0, weights.size()).
     */
    size_t SelectWeighted(const std::vector<float> &weights) {
        if (weights.empty())
            return 0;

        float total = 0.0f;
        for (float w : weights) {
            total += w;
        }

        if (total <= 0.0f)
            return 0;

        float target = NextFloat() * total;
        float cumulative = 0.0f;

        for (size_t i = 0; i < weights.size(); ++i) {
            cumulative += weights[i];
            if (target < cumulative) {
                return i;
            }
        }

        return weights.size() - 1;  // Fallback for floating-point edge cases
    }

    /**
     * @brief Returns a random boolean with given probability of true.
     *
     * @param probability Probability of returning true [0, 1].
     * @return True with the given probability.
     */
    bool NextBool(float probability = 0.5f) {
        return NextFloat() < probability;
    }

    /**
     * @brief Shuffles a vector in-place using Fisher-Yates algorithm.
     *
     * @tparam T The element type.
     * @param vec The vector to shuffle.
     */
    template <typename T>
    void Shuffle(std::vector<T> &vec) {
        for (size_t i = vec.size(); i > 1; --i) {
            size_t j = NextBounded(static_cast<uint32_t>(i));
            std::swap(vec[i - 1], vec[j]);
        }
    }

    /**
     * @brief Gets the current internal state (for serialization/debugging).
     *
     * @return The current 64-bit state.
     */
    uint64_t GetState() const {
        return state_;
    }

    /**
     * @brief Gets the current increment/stream value.
     *
     * @return The current increment value.
     */
    uint64_t GetIncrement() const {
        return increment_;
    }

    /**
     * @brief Restores state from previously saved values (for save/load).
     *
     * @param state The state to restore.
     * @param increment The increment to restore.
     */
    void RestoreState(uint64_t state, uint64_t increment) {
        state_ = state;
        increment_ = increment;
    }

 private:
    /// PCG multiplier constant
    static constexpr uint64_t kMultiplier = 6364136223846793005ULL;

    uint64_t state_;      ///< Current generator state
    uint64_t increment_;  ///< Stream selector (must be odd)
};

}  // namespace worldgen
