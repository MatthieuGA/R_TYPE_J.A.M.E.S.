#include <gtest/gtest.h>
#include "include/sparse_array.hpp"
#include <string>

// Test fixture for sparse_array tests
template <typename T>
class SparseArrayTest : public ::testing::Test {
protected:
    Engine::sparse_array<T> array;
};

// Test with different types
using TestTypes = ::testing::Types<int, double, std::string>;
TYPED_TEST_SUITE(SparseArrayTest, TestTypes);

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

TYPED_TEST(SparseArrayTest, DefaultConstructor) {
    EXPECT_EQ(this->array.size(), 0);
}

TYPED_TEST(SparseArrayTest, CopyConstructor) {
    this->array.insert_at(0, TypeParam{});
    this->array.insert_at(2, TypeParam{});

    Engine::sparse_array<TypeParam> copy(this->array);

    EXPECT_EQ(copy.size(), this->array.size());
    EXPECT_TRUE(copy.has(0));
    EXPECT_FALSE(copy.has(1));
    EXPECT_TRUE(copy.has(2));
}

TYPED_TEST(SparseArrayTest, MoveConstructor) {
    this->array.insert_at(0, TypeParam{});
    auto size = this->array.size();

    Engine::sparse_array<TypeParam> moved(std::move(this->array));

    EXPECT_EQ(moved.size(), size);
    EXPECT_TRUE(moved.has(0));
}

// ============================================================================
// ASSIGNMENT OPERATOR TESTS
// ============================================================================

TYPED_TEST(SparseArrayTest, CopyAssignment) {
    this->array.insert_at(0, TypeParam{});

    Engine::sparse_array<TypeParam> copy;
    copy = this->array;

    EXPECT_EQ(copy.size(), this->array.size());
    EXPECT_TRUE(copy.has(0));
}

TYPED_TEST(SparseArrayTest, MoveAssignment) {
    this->array.insert_at(0, TypeParam{});
    auto size = this->array.size();

    Engine::sparse_array<TypeParam> moved;
    moved = std::move(this->array);

    EXPECT_EQ(moved.size(), size);
    EXPECT_TRUE(moved.has(0));
}

TYPED_TEST(SparseArrayTest, SelfAssignment) {
    this->array.insert_at(0, TypeParam{});
    auto size = this->array.size();

    this->array = this->array;

    EXPECT_EQ(this->array.size(), size);
    EXPECT_TRUE(this->array.has(0));
}

// ============================================================================
// INSERT/EMPLACE TESTS
// ============================================================================

TEST(SparseArrayIntTest, InsertAtRvalue) {
    Engine::sparse_array<int> array;

    auto& ref = array.insert_at(0, 42);

    EXPECT_EQ(ref, 42);
    EXPECT_EQ(array[0], 42);
    EXPECT_TRUE(array.has(0));
}

TEST(SparseArrayIntTest, InsertAtLvalue) {
    Engine::sparse_array<int> array;
    int value = 99;

    auto& ref = array.insert_at(0, value);

    EXPECT_EQ(ref, 99);
    EXPECT_EQ(array[0], 99);
}

TEST(SparseArrayIntTest, InsertAtNonSequential) {
    Engine::sparse_array<int> array;

    array.insert_at(5, 50);

    EXPECT_EQ(array.size(), 6);
    EXPECT_TRUE(array.has(5));
    EXPECT_FALSE(array.has(0));
    EXPECT_FALSE(array.has(4));
}

TEST(SparseArrayIntTest, EmplaceAt) {
    Engine::sparse_array<std::string> array;

    auto& ref = array.emplace_at(0, "Hello");

    EXPECT_EQ(ref, "Hello");
    EXPECT_EQ(array[0], "Hello");
}

TEST(SparseArrayIntTest, EmplaceAtWithMultipleArgs) {
    Engine::sparse_array<std::string> array;

    auto& ref = array.emplace_at(0, 5, 'A');  // "AAAAA"

    EXPECT_EQ(ref, "AAAAA");
}

TEST(SparseArrayIntTest, OverwriteExisting) {
    Engine::sparse_array<int> array;

    array.insert_at(0, 10);
    array.insert_at(0, 20);

    EXPECT_EQ(array[0], 20);
}

// ============================================================================
// ERASE TESTS
// ============================================================================

TEST(SparseArrayIntTest, Erase) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 42);

    EXPECT_TRUE(array.has(0));

    array.erase(0);

    EXPECT_FALSE(array.has(0));
}

TEST(SparseArrayIntTest, EraseNonExistent) {
    Engine::sparse_array<int> array;

    // Should not throw or crash
    EXPECT_NO_THROW(array.erase(100));
}

TEST(SparseArrayIntTest, EraseDoesNotShrinkSize) {
    Engine::sparse_array<int> array;
    array.insert_at(5, 42);
    auto size = array.size();

    array.erase(5);

    EXPECT_EQ(array.size(), size);
    EXPECT_FALSE(array.has(5));
}

// ============================================================================
// HAS/SIZE TESTS
// ============================================================================

TEST(SparseArrayIntTest, HasEmpty) {
    Engine::sparse_array<int> array;
    EXPECT_FALSE(array.has(0));
    EXPECT_FALSE(array.has(100));
}

TEST(SparseArrayIntTest, HasAfterInsert) {
    Engine::sparse_array<int> array;
    array.insert_at(3, 42);

    EXPECT_FALSE(array.has(0));
    EXPECT_FALSE(array.has(2));
    EXPECT_TRUE(array.has(3));
    EXPECT_FALSE(array.has(4));
}

TEST(SparseArrayIntTest, SizeGrowth) {
    Engine::sparse_array<int> array;

    EXPECT_EQ(array.size(), 0);

    array.insert_at(0, 1);
    EXPECT_EQ(array.size(), 1);

    array.insert_at(5, 2);
    EXPECT_EQ(array.size(), 6);
}

// ============================================================================
// OPERATOR[] TESTS
// ============================================================================

TEST(SparseArrayIntTest, SubscriptOperator) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 42);

    EXPECT_EQ(array[0], 42);
}

TEST(SparseArrayIntTest, SubscriptOperatorConst) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 42);

    const auto& const_array = array;
    EXPECT_EQ(const_array[0], 42);
}

TEST(SparseArrayIntTest, SubscriptOperatorModify) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 10);

    array[0] = 20;

    EXPECT_EQ(array[0], 20);
}

// ============================================================================
// ITERATOR TESTS
// ============================================================================

TEST(SparseArrayIntTest, BeginEnd) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 1);
    array.insert_at(1, 2);

    auto it = array.begin();
    EXPECT_NE(it, array.end());
}

TEST(SparseArrayIntTest, IteratorTraversal) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 10);
    array.insert_at(1, 20);
    array.insert_at(2, 30);

    int count = 0;
    for (auto it = array.begin(); it != array.end(); ++it) {
        count++;
    }

    EXPECT_EQ(count, 3);
}

TEST(SparseArrayIntTest, ConstIterator) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 42);

    const auto& const_array = array;
    auto it = const_array.begin();

    EXPECT_NE(it, const_array.end());
}

TEST(SparseArrayIntTest, CBeginCEnd) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 42);

    auto it = array.cbegin();
    EXPECT_NE(it, array.cend());
}

// ============================================================================
// GET_INDEX TESTS
// ============================================================================

TEST(SparseArrayIntTest, GetIndex) {
    Engine::sparse_array<int> array;
    auto& ref = array.insert_at(5, 42);

    auto idx = array.get_index(ref);

    EXPECT_EQ(idx, 5);
}

TEST(SparseArrayIntTest, GetIndexNotFound) {
    Engine::sparse_array<int> array;
    array.insert_at(0, 42);

    int external = 99;
    auto idx = array.get_index(external);

    EXPECT_EQ(idx, static_cast<size_t>(-1));
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST(SparseArrayIntTest, LargeIndices) {
    Engine::sparse_array<int> array;

    array.insert_at(1000, 42);

    EXPECT_EQ(array.size(), 1001);
    EXPECT_TRUE(array.has(1000));
    EXPECT_EQ(array[1000], 42);
}

TEST(SparseArrayIntTest, SparsePattern) {
    Engine::sparse_array<int> array;

    array.insert_at(0, 1);
    array.insert_at(5, 2);
    array.insert_at(10, 3);

    EXPECT_TRUE(array.has(0));
    EXPECT_FALSE(array.has(1));
    EXPECT_FALSE(array.has(4));
    EXPECT_TRUE(array.has(5));
    EXPECT_FALSE(array.has(9));
    EXPECT_TRUE(array.has(10));
}

TEST(SparseArrayStringTest, ComplexType) {
    Engine::sparse_array<std::string> array;

    array.insert_at(0, "Hello");
    array.insert_at(1, "World");

    EXPECT_EQ(array[0], "Hello");
    EXPECT_EQ(array[1], "World");

    array[0] = "Modified";
    EXPECT_EQ(array[0], "Modified");
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST(SparseArrayIntTest, MultipleInsertionsAndErasures) {
    Engine::sparse_array<int> array;

    for (size_t i = 0; i < 100; ++i) {
        array.insert_at(i, static_cast<int>(i * 10));
    }

    EXPECT_EQ(array.size(), 100);

    for (size_t i = 0; i < 100; i += 2) {
        array.erase(i);
    }

    for (size_t i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            EXPECT_FALSE(array.has(i));
        } else {
            EXPECT_TRUE(array.has(i));
            EXPECT_EQ(array[i], static_cast<int>(i * 10));
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
