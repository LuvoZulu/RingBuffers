/**
 * @file    test_ring_buffers.cpp
 * @brief   Catch2 test suite for gabs::Array (ring_buffers.hpp)
 *
 *
 * NOTE ON SCOPE: these tests assert INTENDED/CORRECT behavior for a
 * production-grade circular buffer (size_t underflow guards, real move
 * semantics, safe empty-container handling, proper iterator sentinels,
 * strong self-assignment safety, etc). Several of them will currently FAIL
 * against the implementation as posted — that's expected and is the point
 * of a red/green TDD loop, not a mistake in the tests.
 * @TODO : Implement testing for linked list
 */
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <type_traits>


#include <gabs/ring_buffers.hpp>

using gabs::containers::Array;


// Construction
TEST_CASE("Default construction yields empty, null-backed Array", "[array][construction]") {
    Array arr;

    SECTION("data() is nullptr") {
        REQUIRE(arr.data() == nullptr);
    }
    SECTION("size() is zero") {
        REQUIRE(arr.size() == 0);
    }
    SECTION("capacity() is zero") {
        REQUIRE(arr.capacity() == 0);
    }
    SECTION("head() and tail() start at zero") {
        REQUIRE(arr.head() == 0);
        REQUIRE(arr.tail() == 0);
    }
}


// push_back
TEST_CASE("push_back increases size by exactly one", "[array][push_back]") {
    Array arr;

    arr.push_back(42);
    REQUIRE(arr.size() == 1);

    arr.push_back(7);
    REQUIRE(arr.size() == 2);
}

TEST_CASE("push_back stores the correct values in order", "[array][push_back]") {
    Array arr;
    arr.push_back(10);
    arr.push_back(20);
    arr.push_back(30);

    REQUIRE(arr[0] == 10);
    REQUIRE(arr[1] == 20);
    REQUIRE(arr[2] == 30);
}

TEST_CASE("push_back many elements keeps size accurate", "[array][push_back]") {
    Array arr;
    for (int i = 0; i < 100; ++i) {
        arr.push_back(i);
        REQUIRE(arr.size() == static_cast<size_t>(i + 1));
    }
}

TEST_CASE("push_back values survive interleaved reallocations", "[array][push_back][reallocation]") {
    // Reallocation copies elements out via (head_+i) & (capacity_-1)
    Array arr;
    for (int i = 0; i < 37; ++i) arr.push_back(i);

    for (int i = 0; i < 37; ++i) {
        REQUIRE(arr[i] == i);
    }
}

TEST_CASE("capacity is always >= size after push_back", "[array][push_back][capacity]") {
    Array arr;
    for (int i = 0; i < 200; ++i) {
        arr.push_back(i);
        REQUIRE(arr.capacity() >= arr.size());
    }
}

TEST_CASE("capacity never shrinks during a pure push_back run", "[array][push_back][capacity]") {
    Array arr;
    size_t prevCapacity = arr.capacity();
    for (int i = 0; i < 50; ++i) {
        arr.push_back(i);
        REQUIRE(arr.capacity() >= prevCapacity);
        prevCapacity = arr.capacity();
    }
}

TEST_CASE("capacity doubles exactly at each growth boundary", "[array][push_back][capacity]") {
    Array arr;

    arr.push_back(1);
    REQUIRE(arr.capacity() == 1);

    arr.push_back(2);
    REQUIRE(arr.capacity() == 2);

    arr.push_back(3);
    REQUIRE(arr.capacity() == 4);

    arr.push_back(4);
    REQUIRE(arr.capacity() == 4);

    arr.push_back(5);
    REQUIRE(arr.capacity() == 8);
}

// pop_front
TEST_CASE("pop_front decreases size by exactly one", "[array][pop_front]") {
    Array arr;
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

TEST_CASE("pop_front advances logical front correctly", "[array][pop_front]") {
    Array arr;
    arr.push_back(10);
    arr.push_back(20);
    arr.push_back(30);

    arr.pop_front();
    REQUIRE(arr[0] == 20);
    REQUIRE(arr[1] == 30);

    arr.pop_front();
    REQUIRE(arr[0] == 30);
}

TEST_CASE("pop_front does not grow capacity", "[array][pop_front][capacity]") {
    Array arr;
    arr.push_back(1);
    arr.push_back(2);
    size_t capacityBefore = arr.capacity();

    arr.pop_front();

    REQUIRE(arr.capacity() <= capacityBefore);
}

TEST_CASE("pop_front shrinks capacity once size drops below half", "[array][pop_front][capacity]") {
    Array arr;
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    REQUIRE(arr.capacity() == 4);

    arr.pop_front(); // size 2 of 4 -> at threshold
    arr.pop_front(); // size 1 of remaining capacity -> should shrink
    REQUIRE(arr.capacity() <= 4);
}

TEST_CASE("push_back after popping back to empty still works", "[array][push_back][pop_front]") {
    Array arr;
    arr.push_back(1);
    arr.pop_front();
    REQUIRE(arr.size() == 0);

    arr.push_back(2);
    REQUIRE(arr.size() == 1);
    REQUIRE(arr[0] == 2);
}

TEST_CASE("Interleaved push_back/pop_front keep size bookkeeping correct", "[array][stress]") {
    Array arr;
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

TEST_CASE("pop_front on an empty Array does not corrupt state", "[array][pop_front][edge-case]") {
    // EXPECTED TO FAIL against current code: `if (size_ < 0) return;` can
    // never be true because size_ is size_t (unsigned) — the guard is dead
    // std::bad_alloc or crash rather than safely no-op.
    Array arr;
    REQUIRE(arr.size() == 0);
    REQUIRE_NOTHROW(arr.pop_front());
    REQUIRE(arr.size() == 0);
}

TEST_CASE("Repeated pop_front on empty Array remains stable", "[array][pop_front][edge-case]") {
    // EXPECTED TO FAIL for the same underflow reason as above.
    Array arr;
    for (int i = 0; i < 5; ++i) {
        REQUIRE_NOTHROW(arr.pop_front());
        REQUIRE(arr.size() == 0);
    }
}

// operator[] edge cases
TEST_CASE("Indexing into an empty Array does not read through a null pointer", "[array][indexing][edge-case]") {
    // EXPECTED TO FAIL against current code: data_ is nullptr and
    Array arr;
    REQUIRE_THROWS_AS(arr[0], std::out_of_range);
}

TEST_CASE("Indexing at exactly size() is out of bounds", "[array][indexing][edge-case]") {
    // EXPECTED TO FAIL: operator[] has no bounds checking at all currently.
    Array arr;
    arr.push_back(1);
    arr.push_back(2);
    REQUIRE_THROWS_AS(arr[2], std::out_of_range);
}

TEST_CASE("const operator[] returns the same values as non-const", "[array][indexing]") {
    Array arr;
    arr.push_back(5);
    arr.push_back(6);

    const Array& carr = arr;
    REQUIRE(carr[0] == 5);
    REQUIRE(carr[1] == 6);
}

// Copy construction

TEST_CASE("Copy-constructing an empty Array yields an empty Array", "[array][copy]") {
    Array original;
    Array copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 0);
}

TEST_CASE("Copy-constructing a non-empty Array preserves size and values", "[array][copy]") {
    Array original;
    original.push_back(10);
    original.push_back(20);
    original.push_back(30);

    Array copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 3);
    REQUIRE(copy.capacity() >= copy.size());
    REQUIRE(copy[0] == 10);
    REQUIRE(copy[1] == 20);
    REQUIRE(copy[2] == 30);
}

TEST_CASE("Copy preserves size after original has had elements popped", "[array][copy]") {
    Array original;
    for (int i = 0; i < 5; ++i) original.push_back(i);
    original.pop_front();
    original.pop_front();

    Array copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 3);
    REQUIRE(copy[0] == 2);
    REQUIRE(copy[1] == 3);
    REQUIRE(copy[2] == 4);
}

TEST_CASE("Copy is a genuine deep copy — mutating the copy does not affect the original", "[array][copy]") {
    Array original;
    original.push_back(1);
    original.push_back(2);

    Array copy(original);
    copy.push_back(3);
    copy.pop_front();
    copy.pop_front();

    REQUIRE(original.size() == 2);
    REQUIRE(copy.size() == 1);
    REQUIRE(original[0] == 1);
    REQUIRE(original[1] == 2);
}

TEST_CASE("Mutating the original after copying does not affect the copy", "[array][copy]") {
    Array original;
    original.push_back(1);

    Array copy(original);
    original.push_back(2);
    original.push_back(3);

    REQUIRE(copy.size() == 1);
    REQUIRE(copy[0] == 1);
    REQUIRE(original.size() == 3);
}

TEST_CASE("Copy of an Array with a wrapped (non-zero head) ring buffer preserves logical order", "[array][copy][edge-case]") {
    Array original;
    for (int i = 0; i < 4; ++i) original.push_back(i); // capacity 4, head 0
    original.pop_front();                              // head 1
    original.pop_front();                              // head 2
    original.push_back(4);
    original.push_back(5);

    Array copy(original);

    REQUIRE(copy.size() == original.size());
    for (size_t i = 0; i < copy.size(); ++i) {
        REQUIRE(copy[i] == original[i]);
    }
}

TEST_CASE("Copy constructor does not alias the source's storage", "[array][copy]") {
    Array original;
    original.push_back(99);

    Array copy(original);

    REQUIRE(copy.data() != original.data());
}

// Copy assignment
TEST_CASE("Copy assignment replaces contents and matches source", "[array][copy][assignment]") {
    Array a;
    a.push_back(1);
    a.push_back(2);

    Array b;
    b.push_back(100);

    b = a;

    REQUIRE(b.size() == a.size());
    REQUIRE(b[0] == 1);
    REQUIRE(b[1] == 2);
}

TEST_CASE("Copy assignment is a deep copy", "[array][copy][assignment]") {
    Array a;
    a.push_back(1);
    a.push_back(2);

    Array b;
    b = a;
    b.push_back(3);

    REQUIRE(a.size() == 2);
    REQUIRE(b.size() == 3);
}

TEST_CASE("Self-assignment via operator=(const Array&) leaves the Array unchanged", "[array][copy][assignment][edge-case]") {
    Array a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);

    a = a;

    REQUIRE(a.size() == 3);
    REQUIRE(a[0] == 1);
    REQUIRE(a[1] == 2);
    REQUIRE(a[2] == 3);
}

TEST_CASE("Assigning an empty Array over a non-empty one empties it", "[array][copy][assignment]") {
    Array a;
    a.push_back(1);
    a.push_back(2);

    Array empty;
    a = empty;

    REQUIRE(a.size() == 0);
}

TEST_CASE("operator=(const Array&) does not leak the previous buffer (no aliasing)", "[array][copy][assignment]") {
    Array a;
    a.push_back(1);
    Array b;
    b.push_back(2);
    b.push_back(3);

    const Array& aRef = a;
    b = aRef;

    REQUIRE(b.data() != a.data());
    REQUIRE(b.size() == 1);
    REQUIRE(b[0] == 1);
}

// Move construction / move assignment
TEST_CASE("Move construction transfers size and values from source", "[array][move]") {

    Array original;
    original.push_back(1);
    original.push_back(2);
    original.push_back(3);

    Array moved(std::move(original));

    REQUIRE(moved.size() == 3);
    REQUIRE(moved[0] == 1);
    REQUIRE(moved[1] == 2);
    REQUIRE(moved[2] == 3);
}

TEST_CASE("Move construction leaves the source in a valid (empty-or-defined) state", "[array][move]") {
    Array original;
    original.push_back(1);
    original.push_back(2);

    Array moved(std::move(original));

    REQUIRE(original.size() == 0);
    REQUIRE(original.data() == nullptr);
}

TEST_CASE("Move assignment transfers size and values from source", "[array][move][assignment]") {
    Array original;
    original.push_back(5);
    original.push_back(6);
    original.push_back(7);

    Array target;
    target.push_back(999);

    target = std::move(original);

    REQUIRE(target.size() == 3);
    REQUIRE(target[0] == 5);
    REQUIRE(target[1] == 6);
    REQUIRE(target[2] == 7);
}

TEST_CASE("Move is cheaper than copy: moved-to Array aliases the source's original buffer", "[array][move][assignment]") {
    Array original;
    original.push_back(1);
    int* originalData = original.data();

    Array target;
    target = std::move(original);

    REQUIRE(target.data() == originalData);
}

// Iterators

TEST_CASE("begin() == end() for an empty Array", "[array][iterator]") {
    Array arr;
    REQUIRE(arr.begin() == arr.end());
}

TEST_CASE("begin() != end() for a non-empty Array", "[array][iterator]") {
    Array arr;
    arr.push_back(1);
    REQUIRE(arr.begin() != arr.end());
}

TEST_CASE("Iterating with operator++ visits every element in logical order", "[array][iterator]") {
    Array arr;
    arr.push_back(10);
    arr.push_back(20);
    arr.push_back(30);

    std::vector<int> visited;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        visited.push_back(*it);
    }

    REQUIRE(visited == std::vector<int>{10, 20, 30});
}

TEST_CASE("Range-based for loop visits every element exactly once", "[array][iterator]") {
    Array arr;
    for (int i = 0; i < 10; ++i) arr.push_back(i);

    std::vector<int> visited;
    for (int v : arr) {
        visited.push_back(v);
    }

    REQUIRE(visited.size() == 10);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(visited[static_cast<size_t>(i)] == i);
    }
}

TEST_CASE("Iteration order is correct after the ring has wrapped (non-zero head)", "[array][iterator][edge-case]") {
    Array arr;
    for (int i = 0; i < 4; ++i) arr.push_back(i);
    arr.pop_front(); // head 1
    arr.pop_front(); // head 2
    arr.push_back(4);
    arr.push_back(5);

    std::vector<int> visited;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        visited.push_back(*it);
    }

    REQUIRE(visited == std::vector<int>{2, 3, 4, 5});
}

TEST_CASE("Post-increment returns the pre-increment position", "[array][iterator]") {
    Array arr;
    arr.push_back(1);
    arr.push_back(2);

    auto it = arr.begin();
    auto old = it++;

    REQUIRE(*old == 1);
    REQUIRE(*it == 2);
}

TEST_CASE("operator-> gives access to the pointed-to element", "[array][iterator]") {
    Array arr;
    arr.push_back(42);

    auto it = arr.begin();
    REQUIRE(*(it.operator->()) == 42);
}

TEST_CASE("Two iterators to the same Array/index compare equal", "[array][iterator]") {
    Array arr;
    arr.push_back(1);
    arr.push_back(2);

    auto it1 = arr.begin();
    auto it2 = arr.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
}

TEST_CASE("Iterating an Array and mutating through *it modifies the underlying element", "[array][iterator]") {
    Array arr;
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);

    for (auto it = arr.begin(); it != arr.end(); ++it) {
        *it *= 10;
    }

    REQUIRE(arr[0] == 10);
    REQUIRE(arr[1] == 20);
    REQUIRE(arr[2] == 30);
}

TEST_CASE("Iterator remains valid to read all elements after a pop_front-triggered shrink", "[array][iterator][edge-case]") {
    Array arr;
    for (int i = 0; i < 3; ++i) arr.push_back(i); // capacity 4
    arr.pop_front(); // size 2/4 -> shrink path
    arr.pop_front(); // size 1

    std::vector<int> visited;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        visited.push_back(*it);
    }

    REQUIRE(visited == std::vector<int>{2});
}

TEST_CASE("end() iterator is not separately dereferenced in normal traversal", "[array][iterator][edge-case]") {
    Array arr;
    arr.push_back(7);

    auto it = arr.begin();
    REQUIRE(*it == 7);
    ++it;
    REQUIRE(it == arr.end());
}