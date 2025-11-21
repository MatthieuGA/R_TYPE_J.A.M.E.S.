#include <gtest/gtest.h>
#include "include/sparse_array.hpp"
#include "include/zipper.hpp"
#include "include/indexed_zipper.hpp"
#include <string>
#include <vector>
#include <cmath>

// Test components
struct Position {
    float x;
    float y;
    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Velocity {
    float dx;
    float dy;
    Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}
    bool operator==(const Velocity& other) const {
        return dx == other.dx && dy == other.dy;
    }
};

struct Health {
    int hp;
    explicit Health(int hp = 100) : hp(hp) {}
    bool operator==(const Health& other) const {
        return hp == other.hp;
    }
};

// ============================================================================
// ZIPPER ITERATOR TESTS
// ============================================================================

TEST(ZipperIteratorTest, Construction) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 2.0f});
    velocities.insert_at(0, Velocity{3.0f, 4.0f});

    auto zipper = Engine::make_zipper(positions, velocities);
    auto it = zipper.begin();

    EXPECT_NE(it, zipper.end());
}

TEST(ZipperIteratorTest, Dereference) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{10.0f, 20.0f});
    velocities.insert_at(0, Velocity{5.0f, 10.0f});

    auto zipper = Engine::make_zipper(positions, velocities);
    auto it = zipper.begin();

    auto [pos, vel] = *it;

    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
    EXPECT_EQ(vel.dx, 5.0f);
    EXPECT_EQ(vel.dy, 10.0f);
}

TEST(ZipperIteratorTest, PreIncrement) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});
    positions.insert_at(1, Position{2.0f, 2.0f});
    velocities.insert_at(1, Velocity{2.0f, 2.0f});

    auto zipper = Engine::make_zipper(positions, velocities);
    auto it = zipper.begin();

    ++it;

    auto [pos, vel] = *it;
    EXPECT_EQ(pos.x, 2.0f);
    EXPECT_EQ(vel.dx, 2.0f);
}

TEST(ZipperIteratorTest, PostIncrement) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});
    positions.insert_at(1, Position{2.0f, 2.0f});
    velocities.insert_at(1, Velocity{2.0f, 2.0f});

    auto zipper = Engine::make_zipper(positions, velocities);
    auto it = zipper.begin();

    auto old_it = it++;

    auto [old_pos, old_vel] = *old_it;
    auto [new_pos, new_vel] = *it;

    EXPECT_EQ(old_pos.x, 1.0f);
    EXPECT_EQ(new_pos.x, 2.0f);
}

TEST(ZipperIteratorTest, Equality) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    auto zipper = Engine::make_zipper(positions, velocities);
    auto it1 = zipper.begin();
    auto it2 = zipper.begin();

    EXPECT_EQ(it1, it2);
}

TEST(ZipperIteratorTest, Inequality) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    auto zipper = Engine::make_zipper(positions, velocities);
    auto it1 = zipper.begin();
    auto it2 = zipper.end();

    EXPECT_NE(it1, it2);
}

// ============================================================================
// ZIPPER TESTS
// ============================================================================

TEST(ZipperTest, EmptyContainers) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    auto zipper = Engine::make_zipper(positions, velocities);

    EXPECT_EQ(zipper.begin(), zipper.end());
}

TEST(ZipperTest, SingleElement) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{5.0f, 10.0f});
    velocities.insert_at(0, Velocity{1.0f, 2.0f});

    auto zipper = Engine::make_zipper(positions, velocities);

    int count = 0;
    for (auto [pos, vel] : zipper) {
        // Components are now direct references
        EXPECT_EQ(pos.x, 5.0f);
        EXPECT_EQ(vel.dx, 1.0f);
        count++;
    }

    EXPECT_EQ(count, 1);
}

TEST(ZipperTest, MultipleElements) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    for (int i = 0; i < 5; ++i) {
        positions.insert_at(i, Position{static_cast<float>(i), static_cast<float>(i * 2)});
        velocities.insert_at(i, Velocity{static_cast<float>(i * 3), static_cast<float>(i * 4)});
    }

    auto zipper = Engine::make_zipper(positions, velocities);

    int count = 0;
    for (auto [pos, vel] : zipper) {
        // Components are now direct references - no need to check has_value
        count++;
    }

    EXPECT_EQ(count, 5);
}

TEST(ZipperTest, SkipsMissingComponents) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    // Entity 0: has both
    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    // Entity 1: has only position
    positions.insert_at(1, Position{2.0f, 2.0f});

    // Entity 2: has only velocity
    velocities.insert_at(2, Velocity{3.0f, 3.0f});

    // Entity 3: has both
    positions.insert_at(3, Position{4.0f, 4.0f});
    velocities.insert_at(3, Velocity{4.0f, 4.0f});

    auto zipper = Engine::make_zipper(positions, velocities);

    std::vector<float> x_values;
    for (auto [pos, vel] : zipper) {
        // Components are now direct references
        x_values.push_back(pos.x);
    }

    EXPECT_EQ(x_values.size(), 2);
    EXPECT_EQ(x_values[0], 1.0f);
    EXPECT_EQ(x_values[1], 4.0f);
}

TEST(ZipperTest, ThreeContainers) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;
    Engine::sparse_array<Health> healths;

    // Only entity 1 has all three components
    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    positions.insert_at(1, Position{2.0f, 2.0f});
    velocities.insert_at(1, Velocity{2.0f, 2.0f});
    healths.insert_at(1, Health{100});

    positions.insert_at(2, Position{3.0f, 3.0f});
    healths.insert_at(2, Health{50});

    auto zipper = Engine::make_zipper(positions, velocities, healths);

    int count = 0;
    for (auto [pos, vel, hp] : zipper) {
        // Components are now direct references
        EXPECT_EQ(pos.x, 2.0f);
        EXPECT_EQ(hp.hp, 100);
        count++;
    }

    EXPECT_EQ(count, 1);
}

TEST(ZipperTest, ModifyThroughZipper) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{0.0f, 0.0f});
    velocities.insert_at(0, Velocity{5.0f, 10.0f});

    positions.insert_at(1, Position{10.0f, 20.0f});
    velocities.insert_at(1, Velocity{1.0f, 2.0f});

    // Apply velocity to position - using non-const zipper
    for (auto [pos, vel] : Engine::make_zipper(positions, velocities)) {
        // Components are now direct references
        pos.x += vel.dx;
        pos.y += vel.dy;
    }

    EXPECT_EQ(positions[0]->x, 5.0f);
    EXPECT_EQ(positions[0]->y, 10.0f);
    EXPECT_EQ(positions[1]->x, 11.0f);
    EXPECT_EQ(positions[1]->y, 22.0f);
}

TEST(ZipperTest, RangeBasedFor) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    for (int i = 0; i < 3; ++i) {
        positions.insert_at(i, Position{static_cast<float>(i), 0.0f});
        velocities.insert_at(i, Velocity{static_cast<float>(i * 10), 0.0f});
    }

    int iterations = 0;
    for (auto [pos, vel] : Engine::make_zipper(positions, velocities)) {
        // Components are now direct references
        iterations++;
    }

    EXPECT_EQ(iterations, 3);
}

TEST(ZipperTest, SparseData) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    // Sparse indices: 0, 5, 10, 100
    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    positions.insert_at(5, Position{2.0f, 2.0f});
    velocities.insert_at(5, Velocity{2.0f, 2.0f});

    positions.insert_at(10, Position{3.0f, 3.0f});
    velocities.insert_at(10, Velocity{3.0f, 3.0f});

    positions.insert_at(100, Position{4.0f, 4.0f});
    velocities.insert_at(100, Velocity{4.0f, 4.0f});

    auto zipper = Engine::make_zipper(positions, velocities);

    std::vector<float> x_values;
    for (auto [pos, vel] : zipper) {
        x_values.push_back(pos.x);
    }

    EXPECT_EQ(x_values.size(), 4);
    EXPECT_EQ(x_values[0], 1.0f);
    EXPECT_EQ(x_values[1], 2.0f);
    EXPECT_EQ(x_values[2], 3.0f);
    EXPECT_EQ(x_values[3], 4.0f);
}

// ============================================================================
// INDEXED ZIPPER TESTS
// ============================================================================

TEST(IndexedZipperTest, Construction) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 2.0f});
    velocities.insert_at(0, Velocity{3.0f, 4.0f});

    auto zipper = Engine::make_indexed_zipper(positions, velocities);
    auto it = zipper.begin();

    EXPECT_NE(it, zipper.end());
}

TEST(IndexedZipperTest, ReturnsIndex) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(5, Position{10.0f, 20.0f});
    velocities.insert_at(5, Velocity{5.0f, 10.0f});

    auto zipper = Engine::make_indexed_zipper(positions, velocities);

    for (auto [idx, pos, vel] : zipper) {
        EXPECT_EQ(idx, 5);
        EXPECT_EQ(pos.x, 10.0f);
    }
}

TEST(IndexedZipperTest, MultipleIndices) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    positions.insert_at(2, Position{2.0f, 2.0f});
    velocities.insert_at(2, Velocity{2.0f, 2.0f});

    positions.insert_at(5, Position{3.0f, 3.0f});
    velocities.insert_at(5, Velocity{3.0f, 3.0f});

    auto zipper = Engine::make_indexed_zipper(positions, velocities);

    std::vector<size_t> indices;
    for (auto [idx, pos, vel] : zipper) {
        indices.push_back(idx);
    }

    EXPECT_EQ(indices.size(), 3);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 2);
    EXPECT_EQ(indices[2], 5);
}

TEST(IndexedZipperTest, SkipsMissingComponents) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 1.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    positions.insert_at(1, Position{2.0f, 2.0f});
    // No velocity at index 1

    positions.insert_at(2, Position{3.0f, 3.0f});
    velocities.insert_at(2, Velocity{3.0f, 3.0f});

    auto zipper = Engine::make_indexed_zipper(positions, velocities);

    std::vector<size_t> indices;
    for (auto [idx, pos, vel] : zipper) {
        indices.push_back(idx);
    }

    EXPECT_EQ(indices.size(), 2);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 2);
}

TEST(IndexedZipperTest, ThreeContainers) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;
    Engine::sparse_array<Health> healths;

    positions.insert_at(10, Position{1.0f, 1.0f});
    velocities.insert_at(10, Velocity{1.0f, 1.0f});
    healths.insert_at(10, Health{100});

    positions.insert_at(20, Position{2.0f, 2.0f});
    velocities.insert_at(20, Velocity{2.0f, 2.0f});
    healths.insert_at(20, Health{50});

    auto zipper = Engine::make_indexed_zipper(positions, velocities, healths);

    std::vector<size_t> indices;
    std::vector<int> health_values;

    for (auto [idx, pos, vel, hp] : zipper) {
        indices.push_back(idx);
        health_values.push_back(hp.hp);
    }

    EXPECT_EQ(indices.size(), 2);
    EXPECT_EQ(indices[0], 10);
    EXPECT_EQ(indices[1], 20);
    EXPECT_EQ(health_values[0], 100);
    EXPECT_EQ(health_values[1], 50);
}

TEST(IndexedZipperTest, ModifyUsingIndex) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{0.0f, 0.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});

    positions.insert_at(1, Position{10.0f, 10.0f});
    velocities.insert_at(1, Velocity{2.0f, 2.0f});

    // Scale velocity based on index
    for (auto [idx, pos, vel] : Engine::make_indexed_zipper(positions, velocities)) {
        vel.dx *= static_cast<float>(idx + 1);
        vel.dy *= static_cast<float>(idx + 1);
    }

    EXPECT_EQ(velocities[0]->dx, 1.0f);  // 1.0 * 1
    EXPECT_EQ(velocities[0]->dy, 1.0f);
    EXPECT_EQ(velocities[1]->dx, 4.0f);  // 2.0 * 2
    EXPECT_EQ(velocities[1]->dy, 4.0f);
}

TEST(IndexedZipperTest, EmptyContainers) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    auto zipper = Engine::make_indexed_zipper(positions, velocities);

    EXPECT_EQ(zipper.begin(), zipper.end());
}

TEST(IndexedZipperTest, LargeIndices) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(1000, Position{1.0f, 1.0f});
    velocities.insert_at(1000, Velocity{1.0f, 1.0f});

    positions.insert_at(5000, Position{2.0f, 2.0f});
    velocities.insert_at(5000, Velocity{2.0f, 2.0f});

    auto zipper = Engine::make_indexed_zipper(positions, velocities);

    std::vector<size_t> indices;
    for (auto [idx, pos, vel] : zipper) {
        indices.push_back(idx);
    }

    EXPECT_EQ(indices.size(), 2);
    EXPECT_EQ(indices[0], 1000);
    EXPECT_EQ(indices[1], 5000);
}

// ============================================================================
// INTEGRATION TESTS - SYSTEM-LIKE USAGE
// ============================================================================

TEST(ZipperIntegrationTest, MovementSystem) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;


    for (int i = 0; i < 5; ++i) {
        positions.insert_at(i, Position{0.0f, 0.0f});
        velocities.insert_at(i, Velocity{
            static_cast<float>(i),
            static_cast<float>(i * 2)
        });
    }

    for (auto [pos, vel] : Engine::make_zipper(positions, velocities)) {
        pos.x += vel.dx;
        pos.y += vel.dy;
    }

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(positions[i]->x, static_cast<float>(i));
        EXPECT_EQ(positions[i]->y, static_cast<float>(i * 2));
    }
}

TEST(ZipperIntegrationTest, DamageSystem) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Health> healths;

    positions.insert_at(0, Position{5.0f, 5.0f});   // In range
    healths.insert_at(0, Health{100});

    positions.insert_at(1, Position{15.0f, 15.0f}); // Out of range
    healths.insert_at(1, Health{100});

    positions.insert_at(2, Position{3.0f, 3.0f});   // In range
    healths.insert_at(2, Health{75});

    // Damage system: entities within 10 units of origin take damage
    for (auto [idx, pos, hp] : Engine::make_indexed_zipper(positions, healths)) {
        float distance = std::sqrt(pos.x * pos.x + pos.y * pos.y);
        if (distance < 10.0f) {
            hp.hp -= 10;
        }
    }

    EXPECT_EQ(healths[0]->hp, 90);
    EXPECT_EQ(healths[1]->hp, 100);
    EXPECT_EQ(healths[2]->hp, 65);
}

TEST(ZipperIntegrationTest, LoggingSystem) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    positions.insert_at(0, Position{1.0f, 2.0f});
    velocities.insert_at(0, Velocity{3.0f, 4.0f});

    positions.insert_at(1, Position{5.0f, 6.0f});
    velocities.insert_at(1, Velocity{7.0f, 8.0f});

    std::vector<std::string> logs;

    // Logging system
    for (auto [idx, pos, vel] : Engine::make_indexed_zipper(positions, velocities)) {
        std::string log = "Entity " + std::to_string(idx) +
                         ": pos=(" + std::to_string(pos.x) + "," + std::to_string(pos.y) + ")";
        logs.push_back(log);
    }

    EXPECT_EQ(logs.size(), 2);
}

TEST(ZipperIntegrationTest, ComplexFilteringSystem) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;
    Engine::sparse_array<Health> healths;

    // Entity 0: full setup
    positions.insert_at(0, Position{10.0f, 10.0f});
    velocities.insert_at(0, Velocity{1.0f, 1.0f});
    healths.insert_at(0, Health{50});

    // Entity 1: no health (should be skipped)
    positions.insert_at(1, Position{20.0f, 20.0f});
    velocities.insert_at(1, Velocity{2.0f, 2.0f});

    // Entity 2: full setup
    positions.insert_at(2, Position{30.0f, 30.0f});
    velocities.insert_at(2, Velocity{3.0f, 3.0f});
    healths.insert_at(2, Health{100});

    // Entity 3: no velocity (should be skipped)
    positions.insert_at(3, Position{40.0f, 40.0f});
    healths.insert_at(3, Health{75});

    int processed = 0;
    for (auto [idx, pos, vel, hp] : Engine::make_indexed_zipper(positions, velocities, healths)) {
        // Only entities with all three components - zipper already filtered them
        processed++;
    }

    EXPECT_EQ(processed, 2);  // Only entities 0 and 2
}

TEST(ZipperIntegrationTest, StressTestManyEntities) {
    Engine::sparse_array<Position> positions;
    Engine::sparse_array<Velocity> velocities;

    const size_t num_entities = 1000;

    // Create entities with both components
    for (size_t i = 0; i < num_entities; ++i) {
        if (i % 2 == 0) {  // Only even indices have both
            positions.insert_at(i, Position{
                static_cast<float>(i),
                static_cast<float>(i)
            });
            velocities.insert_at(i, Velocity{1.0f, 1.0f});
        }
    }

    size_t count = 0;
    for (auto [pos, vel] : Engine::make_zipper(positions, velocities)) {
        count++;
    }

    EXPECT_EQ(count, num_entities / 2);
}
