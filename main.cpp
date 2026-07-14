#include "ring_buffers.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Default-constructed Array is empty", "[array][construction]") {
    gabs::Array arr;

    REQUIRE(arr.size() == 0);
    REQUIRE(arr.capacity() == 0);
}

TEST_CASE("size() and capacity() are callable through a const reference", "[array][const-correctness]") {
    gabs::Array arr;
    const gabs::Array& constArr = arr;

    REQUIRE(constArr.size() == 0);
    REQUIRE(constArr.capacity() == 0);
}

TEST_CASE("push_back increases size by exactly one", "[array][push_back]") {
    gabs::Array arr;

    arr.push_back(42);
    REQUIRE(arr.size() == 1);

    arr.push_back(7);
    REQUIRE(arr.size() == 2);
}

TEST_CASE("push_back many elements keeps size accurate", "[array][push_back]") {
    gabs::Array arr;

    for (int i = 0; i < 100; ++i) {
        arr.push_back(i);
        REQUIRE(arr.size() == static_cast<size_t>(i + 1));
    }
}

TEST_CASE("capacity is always >= size after push_back", "[array][push_back][capacity]") {
    gabs::Array arr;

    for (int i = 0; i < 200; ++i) {
        arr.push_back(i);
        REQUIRE(arr.capacity() >= arr.size());
    }
}

TEST_CASE("capacity never shrinks on its own during push_back", "[array][push_back][capacity]") {
    gabs::Array arr;
    size_t prevCapacity = arr.capacity();

    for (int i = 0; i < 50; ++i) {
        arr.push_back(i);
        REQUIRE(arr.capacity() >= prevCapacity);
        prevCapacity = arr.capacity();
    }
}

TEST_CASE("pop_front decreases size by exactly one", "[array][pop_front]") {
    gabs::Array arr;
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    REQUIRE(arr.size() == 3);

    arr.pop_front();
    REQUIRE(arr.size() == 2);

    arr.pop_front();
    REQUIRE(arr.size() == 1);

    arr.pop_front();
    REQUIRE(arr.size() == 0);
}

TEST_CASE("pop_front does not reduce capacity", "[array][pop_front][capacity]") {
    gabs::Array arr;
    arr.push_back(1);
    arr.push_back(2);
    size_t capacityBefore = arr.capacity();

    arr.pop_front();

    REQUIRE(arr.capacity() == capacityBefore);
}

TEST_CASE("push_back after popping back to empty still works", "[array][push_back][pop_front]") {
    gabs::Array arr;
    arr.push_back(1);
    arr.pop_front();
    REQUIRE(arr.size() == 0);

    arr.push_back(2);
    REQUIRE(arr.size() == 1);
}

TEST_CASE("Interleaved push_back/pop_front keep size bookkeeping correct", "[array][stress]") {
    gabs::Array arr;
    size_t expected = 0;

    for (int i = 0; i < 1000; ++i) {
        arr.push_back(i);
        ++expected;
        REQUIRE(arr.size() == expected);

        if (i % 3 == 0) {
            arr.pop_front();
            --expected;
            REQUIRE(arr.size() == expected);
        }
    }
}

TEST_CASE("Copy-constructing an empty Array yields an empty Array", "[array][copy]") {
    gabs::Array original;
    gabs::Array copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 0);
}

TEST_CASE("Copy-constructing a non-empty Array preserves size", "[array][copy]") {
    gabs::Array original;
    original.push_back(10);
    original.push_back(20);
    original.push_back(30);

    gabs::Array copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 3);
    REQUIRE(copy.capacity() >= copy.size());
}

TEST_CASE("Copy preserves size after original has had elements popped", "[array][copy]") {
    gabs::Array original;
    for (int i = 0; i < 5; ++i) original.push_back(i);
    original.pop_front();
    original.pop_front();

    gabs::Array copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 3);
}

TEST_CASE("Mutating the copy does not affect the original (deep copy)", "[array][copy]") {
    gabs::Array original;
    original.push_back(1);
    original.push_back(2);

    gabs::Array copy(original);
    copy.push_back(3);
    copy.pop_front();
    copy.pop_front();

    REQUIRE(original.size() == 2);
    REQUIRE(copy.size() == 1);
}

TEST_CASE("Mutating the original after copying does not affect the copy", "[array][copy]") {
    gabs::Array original;
    original.push_back(1);

    gabs::Array copy(original);
    original.push_back(2);
    original.push_back(3);

    REQUIRE(copy.size() == 1);
    REQUIRE(original.size() == 3);
}


// To implement basic edge cases on empty arrays
 //TEST_CASE("pop_front on empty Array is a no-op", "[array][pop_front][edge-case]") {
 //    gabs::Array arr;
 //    REQUIRE_NOTHROW(arr.pop_front());
 //    REQUIRE(arr.size() == 0);
 //}

 //TEST_CASE("pop_front on empty Array throws", "[array][pop_front][edge-case]") {
 //    gabs::Array arr;
 //    REQUIRE_THROWS_AS(arr.pop_front(), std::out_of_range);
 //}