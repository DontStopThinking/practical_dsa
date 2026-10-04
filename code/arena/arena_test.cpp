#if TESTS_ENABLED
#include "arena/arena.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Arena construction", "[arena][ctor]")
{
    SECTION("Non-zero size allocates a buffer")
    {
        Arena arena(64);

        CHECK(arena.m_buffer != nullptr);
        CHECK(arena.m_capacity == 64);
        CHECK(arena.m_offset == 0);
    }

    SECTION("Zero size creates an empty arena with no buffer")
    {
        Arena arena(0);

        CHECK(arena.m_buffer == nullptr);
        CHECK(arena.m_capacity == 0);
        CHECK(arena.m_offset == 0);
    }
}

TEST_CASE("Arena with zero capacity cannot allocate", "[arena][allocate]")
{
    Arena arena(0);

    CHECK(arena.allocate(1) == nullptr);
    CHECK(arena.allocate(0) == nullptr);
    CHECK(arena.m_offset == 0);
}

TEST_CASE("Arena basic allocation", "[arena][allocate]")
{
    struct Foo
    {
        int m_foo{1};
        double m_bar{10.0};
    };

    Arena arena(1024 * 1024);

    SECTION("Correctly allocates a struct")
    {
        void* memory = arena.allocate(sizeof(Foo), alignof(Foo));
        REQUIRE(memory != nullptr);

        const Foo* f1 = new (memory) Foo{};
        CHECK(f1->m_foo == 1);
        CHECK(f1->m_bar == 10.0);
    }

    SECTION("Resetting an arena resets its offset but does not free memory")
    {
        void* memory = arena.allocate(sizeof(Foo), alignof(Foo));
        REQUIRE(memory != nullptr);

        arena.reset();

        CHECK(arena.m_offset == 0);
        CHECK(arena.m_buffer != nullptr);
        CHECK(arena.m_capacity == 1024 * 1024);
    }
}

#endif // TESTS_ENABLED
