#if TESTS_ENABLED
#include "pool/pool.h"

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

// Dummy object used to track constructor/destructor execution
struct Dummy
{
    int m_value{0};
    int m_padding{0};
    static inline int ms_activeInstances{0};

    Dummy(int v) : m_value{v}
    {
        ms_activeInstances++;
    }

    ~Dummy()
    {
        ms_activeInstances--;
    }
};

TEST_CASE("Pool initialization", "[pool]")
{
    Dummy::ms_activeInstances = 0;
    Pool<Dummy> pool(5);

    // Initial pool state should not have any constructed instances
    REQUIRE(Dummy::ms_activeInstances == 0);
}

TEST_CASE("Pool - Spawning and despawning objects", "[pool]")
{
    Dummy::ms_activeInstances = 0;
    Pool<Dummy> pool(3);

    SECTION("Spawn constructs instances correctly")
    {
        Dummy* obj1 = pool.spawn(42);
        REQUIRE(obj1 != nullptr);
        CHECK(obj1->m_value == 42);
        CHECK(Dummy::ms_activeInstances == 1);

        Dummy* obj2 = pool.spawn(100);
        REQUIRE(obj2 != nullptr);
        CHECK(obj2->m_value == 100);
        CHECK(Dummy::ms_activeInstances == 2);

        SECTION("Despawn calls destructor")
        {
            pool.despawn(obj1);
            CHECK(Dummy::ms_activeInstances == 1);

            pool.despawn(obj2);
            CHECK(Dummy::ms_activeInstances == 0);
        }
    }
}

TEST_CASE("Pool's maximum capacity reached", "[pool]")
{
    Dummy::ms_activeInstances = 0;
    Pool<Dummy> pool(2);

    Dummy* obj1 = pool.spawn(1);
    Dummy* obj2 = pool.spawn(2);

    REQUIRE(obj1 != nullptr);
    REQUIRE(obj2 != nullptr);

    SECTION("Spawning beyond capacity should return nullptr")
    {
        Dummy* obj3 = pool.spawn(3);
        CHECK(obj3 == nullptr);
        CHECK(Dummy::ms_activeInstances == 2);
    }

    SECTION("Pool's memory block should be reusable after despawning")
    {
        Dummy* obj3 = pool.spawn(0);
        CHECK(obj3 == nullptr); // Pool is already full, so this spawning should fail

        // Despawn one object
        pool.despawn(obj1);
        CHECK(Dummy::ms_activeInstances == 1);

        obj3 = pool.spawn(3);
        REQUIRE(obj3 != nullptr); // Now pool should have room for more
        CHECK(obj3->m_value == 3);
        CHECK(Dummy::ms_activeInstances == 2);
    }
}

TEST_CASE("Pool benchmarks", "[pool][benchmark]")
{
    struct Entity
    {
        uint64_t m_data[8];
    };

    // Generate a random deletion sequence to simulate churn
    auto getRandomIndices = [](int count) -> std::vector<int>
    {
        std::vector<int> result(count);
        for (int i = 0; i < count; i++)
        {
            result[i] = i;
        }
        std::mt19937 r(4);
        std::ranges::shuffle(result, r);
        return result;
    };

    const std::vector<int> destroyOrder = getRandomIndices(10000);
    std::vector<Entity*> entities(10000);

    BENCHMARK("Benchmark global heap churn")
    {
        // Allocate 10,000 entities
        for (int i = 0; i < 10000; i++)
        {
            entities[i] = new Entity;
        }

        // Destroy them in random order
        for (int i : destroyOrder)
        {
            delete entities[i];
        }
    };

    Pool<Entity> myPool(10000);

    BENCHMARK("Benchmark Pool churn")
    {
        for (int i = 0; i < 10000; i++)
        {
            entities[i] = myPool.spawn();
        }

        // Destroy them in random order
        for (int i : destroyOrder)
        {
            myPool.despawn(entities[i]);
        }
    };
}
#endif // TESTS_ENABLED
