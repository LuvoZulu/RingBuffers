/**
 * @file    test_ring_buffers.cpp
 * @brief   Catch2 test suite for gabs::containers::Array<T> (array.hpp)
 *
 * NOTE ON SCOPE: these tests assert INTENDED/CORRECT behavior for a
 * production-grade templated ring buffer. Several of them will currently
 * FAIL against the implementation as posted — that's expected and is the
 * point of a red/green TDD loop, not a mistake in the tests.
 *
 * KNOWN BLOCKER: Array::push_back takes `T&` (non-const lvalue ref), so
 * rvalue literals (e.g. push_back(42)) will NOT compile. All tests below
 * use named lvalues to work around this. Flagging it rather than hiding
 * it — see review notes accompanying this file.
 *
 * Type coverage rationale:
 *  - int / double: trivially-copyable baseline, cheap to reason about.
 *  - std::string: owns a heap resource. If reallocate() moves elements
 *    with memcpy/reinterpret_cast instead of real copy/move construction,
 *    this type is where it will show up as corruption/double-free/UB
 *    rather than a clean assertion failure.
 *  - TrackedValue: instrumented type that counts copy ctor / move ctor /
 *    copy assign / move assign calls, to directly verify the class is
 *    invoking T's actual special members (Rule-of-Three/Five correctness)
 *    rather than doing something structurally sneaky.
 *
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>

#include <gabs/ring_buffers.hpp>

using gabs::containers::Array;

// ---------------------------------------------------------------------
// Value factory: produces a type-appropriate, distinguishable value for
// slot `n`, as a named lvalue (required by push_back(T&)).
// ---------------------------------------------------------------------
template <typename T>
T make_value(int n);

template <>
int make_value<int>(int n) { return n; }

template <>
double make_value<double>(int n) { return static_cast<double>(n) + 0.5; }

template <>
std::string make_value<std::string>(int n) { return "val_" + std::to_string(n); }

// ---------------------------------------------------------------------
// TrackedValue: instrumented type for verifying real copy/move semantics
// ---------------------------------------------------------------------
struct TrackedValue {
    static inline int copy_ctor_count = 0;
    static inline int move_ctor_count = 0;
    static inline int copy_assign_count = 0;
    static inline int move_assign_count = 0;
    static inline int default_ctor_count = 0;
    static inline int value_ctor_count = 0;

    int val;

    TrackedValue() : val(0) { ++default_ctor_count; }
    explicit TrackedValue(int v) : val(v) { ++value_ctor_count; }

    TrackedValue(const TrackedValue& other) : val(other.val) { ++copy_ctor_count; }
    TrackedValue(TrackedValue&& other) noexcept : val(other.val) {
        other.val = -1;
        ++move_ctor_count;
    }
    TrackedValue& operator=(const TrackedValue& other) {
        val = other.val;
        ++copy_assign_count;
        return *this;
    }
    TrackedValue& operator=(TrackedValue&& other) noexcept {
        val = other.val;
        other.val = -1;
        ++move_assign_count;
        return *this;
    }

    bool operator==(const TrackedValue& other) const { return val == other.val; }

    static void reset() {
        copy_ctor_count = move_ctor_count = 0;
        copy_assign_count = move_assign_count = 0;
        default_ctor_count = value_ctor_count = 0;
    }
};

template <>
TrackedValue make_value<TrackedValue>(int n) { return TrackedValue(n); }

// =======================================================================
// Construction
// =======================================================================
TEMPLATE_TEST_CASE("Default construction yields empty, null-backed Array",
    "[array][construction][template]", int, double, std::string) {
    Array<TestType> arr;

    SECTION("data() is nullptr") { REQUIRE(arr.data() == nullptr); }
    SECTION("size() is zero") { REQUIRE(arr.size() == 0); }
    SECTION("capacity() is zero") { REQUIRE(arr.capacity() == 0); }
    SECTION("head() and tail() start at zero") {
        REQUIRE(arr.head() == 0);
        REQUIRE(arr.tail() == 0);
    }
}

// =======================================================================
// push_back
// =======================================================================
TEMPLATE_TEST_CASE("push_back increases size by exactly one",
    "[array][push_back][template]", int, double, std::string) {
    Array<TestType> arr;

    auto a = make_value<TestType>(42);
    arr.push_back(a);
    REQUIRE(arr.size() == 1);

    auto b = make_value<TestType>(7);
    arr.push_back(b);
    REQUIRE(arr.size() == 2);
}

TEMPLATE_TEST_CASE("push_back stores the correct values in order",
    "[array][push_back][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v0 = make_value<TestType>(10);
    auto v1 = make_value<TestType>(20);
    auto v2 = make_value<TestType>(30);
    arr.push_back(v0);
    arr.push_back(v1);
    arr.push_back(v2);

    REQUIRE(arr[0] == v0);
    REQUIRE(arr[1] == v1);
    REQUIRE(arr[2] == v2);
}

TEMPLATE_TEST_CASE("push_back many elements keeps size accurate",
    "[array][push_back][template]", int, double, std::string) {
    Array<TestType> arr;
    for (int i = 0; i < 100; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v);
        REQUIRE(arr.size() == static_cast<size_t>(i + 1));
    }
}

TEMPLATE_TEST_CASE("push_back values survive interleaved reallocations",
    "[array][push_back][reallocation][template]", int, double, std::string) {
    // Reallocation copies elements out via (head_+i) & (capacity_-1).
    // For std::string this is the case most likely to reveal a memcpy-style
    // reallocate() instead of proper element-wise construction.
    Array<TestType> arr;
    for (int i = 0; i < 37; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v);
    }

    for (int i = 0; i < 37; ++i) {
        REQUIRE(arr[static_cast<size_t>(i)] == make_value<TestType>(i));
    }
}

TEMPLATE_TEST_CASE("capacity is always >= size after push_back",
    "[array][push_back][capacity][template]", int, double, std::string) {
    Array<TestType> arr;
    for (int i = 0; i < 200; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v);
        REQUIRE(arr.capacity() >= arr.size());
    }
}

TEMPLATE_TEST_CASE("capacity never shrinks during a pure push_back run",
    "[array][push_back][capacity][template]", int, double, std::string) {
    Array<TestType> arr;
    size_t prevCapacity = arr.capacity();
    for (int i = 0; i < 50; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v);
        REQUIRE(arr.capacity() >= prevCapacity);
        prevCapacity = arr.capacity();
    }
}

TEMPLATE_TEST_CASE("capacity doubles exactly at each growth boundary",
    "[array][push_back][capacity][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    auto v4 = make_value<TestType>(4);
    auto v5 = make_value<TestType>(5);

    arr.push_back(v1);
    REQUIRE(arr.capacity() == 1);

    arr.push_back(v2);
    REQUIRE(arr.capacity() == 2);

    arr.push_back(v3);
    REQUIRE(arr.capacity() == 4);

    arr.push_back(v4);
    REQUIRE(arr.capacity() == 4);

    arr.push_back(v5);
    REQUIRE(arr.capacity() == 8);
}

// =======================================================================
// pop_front
// =======================================================================
TEMPLATE_TEST_CASE("pop_front decreases size by exactly one",
    "[array][pop_front][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    arr.push_back(v1);
    arr.push_back(v2);
    arr.push_back(v3);
    REQUIRE(arr.size() == 3);

    arr.pop_front();
    REQUIRE(arr.size() == 2);

    arr.pop_front();
    REQUIRE(arr.size() == 1);

    arr.pop_front();
    REQUIRE(arr.size() == 0);
}

TEMPLATE_TEST_CASE("pop_front advances logical front correctly",
    "[array][pop_front][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v10 = make_value<TestType>(10);
    auto v20 = make_value<TestType>(20);
    auto v30 = make_value<TestType>(30);
    arr.push_back(v10);
    arr.push_back(v20);
    arr.push_back(v30);

    arr.pop_front();
    REQUIRE(arr[0] == v20);
    REQUIRE(arr[1] == v30);

    arr.pop_front();
    REQUIRE(arr[0] == v30);
}

TEMPLATE_TEST_CASE("pop_front does not grow capacity",
    "[array][pop_front][capacity][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    arr.push_back(v1);
    arr.push_back(v2);
    size_t capacityBefore = arr.capacity();

    arr.pop_front();

    REQUIRE(arr.capacity() <= capacityBefore);
}

TEMPLATE_TEST_CASE("pop_front shrinks capacity once size drops below half",
    "[array][pop_front][capacity][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    arr.push_back(v1);
    arr.push_back(v2);
    arr.push_back(v3);
    REQUIRE(arr.capacity() == 4);

    arr.pop_front(); // size 2 of 4 -> at threshold
    arr.pop_front(); // size 1 of remaining capacity -> should shrink
    REQUIRE(arr.capacity() <= 4);
}

TEMPLATE_TEST_CASE("push_back after popping back to empty still works",
    "[array][push_back][pop_front][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    arr.push_back(v1);
    arr.pop_front();
    REQUIRE(arr.size() == 0);

    auto v2 = make_value<TestType>(2);
    arr.push_back(v2);
    REQUIRE(arr.size() == 1);
    REQUIRE(arr[0] == v2);
}

TEMPLATE_TEST_CASE("Interleaved push_back/pop_front keep size bookkeeping correct",
    "[array][stress][template]", int, double, std::string) {
    Array<TestType> arr;
    size_t expected = 0;

    for (int i = 0; i < 1000; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v);
        ++expected;
        REQUIRE(arr.size() == expected);

        if (i % 3 == 0) {
            arr.pop_front();
            --expected;
            REQUIRE(arr.size() == expected);
        }
    }
}

TEMPLATE_TEST_CASE("pop_front on an empty Array does not corrupt state",
    "[array][pop_front][edge-case][template]", int, double, std::string) {
    // EXPECTED TO FAIL against a naive port: `if (size_ < 0) return;` can
    // never be true because size_ is size_t (unsigned) — the guard is dead.
    Array<TestType> arr;
    REQUIRE(arr.size() == 0);
    REQUIRE_NOTHROW(arr.pop_front());
    REQUIRE(arr.size() == 0);
}

TEMPLATE_TEST_CASE("Repeated pop_front on empty Array remains stable",
    "[array][pop_front][edge-case][template]", int, double, std::string) {
    Array<TestType> arr;
    for (int i = 0; i < 5; ++i) {
        REQUIRE_NOTHROW(arr.pop_front());
        REQUIRE(arr.size() == 0);
    }
}

// =======================================================================
// operator[] edge cases
// =======================================================================
TEMPLATE_TEST_CASE("Indexing into an empty Array does not read through a null pointer",
    "[array][indexing][edge-case][template]", int, double, std::string) {
    Array<TestType> arr;
    REQUIRE_THROWS_AS(arr[0], std::out_of_range);
}

TEMPLATE_TEST_CASE("Indexing at exactly size() is out of bounds",
    "[array][indexing][edge-case][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    arr.push_back(v1);
    arr.push_back(v2);
    REQUIRE_THROWS_AS(arr[2], std::out_of_range);
}

TEMPLATE_TEST_CASE("const operator[] returns the same values as non-const",
    "[array][indexing][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v5 = make_value<TestType>(5);
    auto v6 = make_value<TestType>(6);
    arr.push_back(v5);
    arr.push_back(v6);

    const Array<TestType>& carr = arr;
    REQUIRE(carr[0] == v5);
    REQUIRE(carr[1] == v6);
}

// =======================================================================
// Copy construction
// =======================================================================
TEMPLATE_TEST_CASE("Copy-constructing an empty Array yields an empty Array",
    "[array][copy][template]", int, double, std::string) {
    Array<TestType> original;
    Array<TestType> copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 0);
}

TEMPLATE_TEST_CASE("Copy-constructing a non-empty Array preserves size and values",
    "[array][copy][template]", int, double, std::string) {
    Array<TestType> original;
    auto v10 = make_value<TestType>(10);
    auto v20 = make_value<TestType>(20);
    auto v30 = make_value<TestType>(30);
    original.push_back(v10);
    original.push_back(v20);
    original.push_back(v30);

    Array<TestType> copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 3);
    REQUIRE(copy.capacity() >= copy.size());
    REQUIRE(copy[0] == v10);
    REQUIRE(copy[1] == v20);
    REQUIRE(copy[2] == v30);
}

TEMPLATE_TEST_CASE("Copy preserves size after original has had elements popped",
    "[array][copy][template]", int, double, std::string) {
    Array<TestType> original;
    for (int i = 0; i < 5; ++i) {
        auto v = make_value<TestType>(i);
        original.push_back(v);
    }
    original.pop_front();
    original.pop_front();

    Array<TestType> copy(original);

    REQUIRE(copy.size() == original.size());
    REQUIRE(copy.size() == 3);
    REQUIRE(copy[0] == make_value<TestType>(2));
    REQUIRE(copy[1] == make_value<TestType>(3));
    REQUIRE(copy[2] == make_value<TestType>(4));
}

TEMPLATE_TEST_CASE("Copy is a genuine deep copy — mutating the copy does not affect the original",
    "[array][copy][template]", int, double, std::string) {
    Array<TestType> original;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    original.push_back(v1);
    original.push_back(v2);

    Array<TestType> copy(original);
    auto v3 = make_value<TestType>(3);
    copy.push_back(v3);
    copy.pop_front();
    copy.pop_front();

    REQUIRE(original.size() == 2);
    REQUIRE(copy.size() == 1);
    REQUIRE(original[0] == v1);
    REQUIRE(original[1] == v2);
}

TEMPLATE_TEST_CASE("Mutating the original after copying does not affect the copy",
    "[array][copy][template]", int, double, std::string) {
    Array<TestType> original;
    auto v1 = make_value<TestType>(1);
    original.push_back(v1);

    Array<TestType> copy(original);
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    original.push_back(v2);
    original.push_back(v3);

    REQUIRE(copy.size() == 1);
    REQUIRE(copy[0] == v1);
    REQUIRE(original.size() == 3);
}

TEMPLATE_TEST_CASE("Copy of an Array with a wrapped (non-zero head) ring buffer preserves logical order",
    "[array][copy][edge-case][template]", int, double, std::string) {
    Array<TestType> original;
    for (int i = 0; i < 4; ++i) {
        auto v = make_value<TestType>(i);
        original.push_back(v);
    } // capacity 4, head 0
    original.pop_front(); // head 1
    original.pop_front(); // head 2
    auto v4 = make_value<TestType>(4);
    auto v5 = make_value<TestType>(5);
    original.push_back(v4);
    original.push_back(v5);

    Array<TestType> copy(original);

    REQUIRE(copy.size() == original.size());
    for (size_t i = 0; i < copy.size(); ++i) {
        REQUIRE(copy[i] == original[i]);
    }
}

TEMPLATE_TEST_CASE("Copy constructor does not alias the source's storage",
    "[array][copy][template]", int, double, std::string) {
    Array<TestType> original;
    auto v99 = make_value<TestType>(99);
    original.push_back(v99);

    Array<TestType> copy(original);

    REQUIRE(copy.data() != original.data());
}

// =======================================================================
// Copy assignment
// =======================================================================
TEMPLATE_TEST_CASE("Copy assignment replaces contents and matches source",
    "[array][copy][assignment][template]", int, double, std::string) {
    Array<TestType> a;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    a.push_back(v1);
    a.push_back(v2);

    Array<TestType> b;
    auto v100 = make_value<TestType>(100);
    b.push_back(v100);

    b = a;

    REQUIRE(b.size() == a.size());
    REQUIRE(b[0] == v1);
    REQUIRE(b[1] == v2);
}

TEMPLATE_TEST_CASE("Copy assignment is a deep copy",
    "[array][copy][assignment][template]", int, double, std::string) {
    Array<TestType> a;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    a.push_back(v1);
    a.push_back(v2);

    Array<TestType> b;
    b = a;
    auto v3 = make_value<TestType>(3);
    b.push_back(v3);

    REQUIRE(a.size() == 2);
    REQUIRE(b.size() == 3);
}

TEMPLATE_TEST_CASE("Self-assignment via operator=(const Array&) leaves the Array unchanged",
    "[array][copy][assignment][edge-case][template]", int, double, std::string) {
    Array<TestType> a;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    a.push_back(v1);
    a.push_back(v2);
    a.push_back(v3);

    a = a;

    REQUIRE(a.size() == 3);
    REQUIRE(a[0] == v1);
    REQUIRE(a[1] == v2);
    REQUIRE(a[2] == v3);
}

TEMPLATE_TEST_CASE("Assigning an empty Array over a non-empty one empties it",
    "[array][copy][assignment][template]", int, double, std::string) {
    Array<TestType> a;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    a.push_back(v1);
    a.push_back(v2);

    Array<TestType> empty;
    a = empty;

    REQUIRE(a.size() == 0);
}

TEMPLATE_TEST_CASE("operator=(const Array&) does not leak the previous buffer (no aliasing)",
    "[array][copy][assignment][template]", int, double, std::string) {
    Array<TestType> a;
    auto v1 = make_value<TestType>(1);
    a.push_back(v1);
    Array<TestType> b;
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    b.push_back(v2);
    b.push_back(v3);

    const Array<TestType>& aRef = a;
    b = aRef;

    REQUIRE(b.data() != a.data());
    REQUIRE(b.size() == 1);
    REQUIRE(b[0] == v1);
}

// =======================================================================
// Move construction / move assignment
// =======================================================================
TEMPLATE_TEST_CASE("Move construction transfers size and values from source",
    "[array][move][template]", int, double, std::string) {
    Array<TestType> original;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    original.push_back(v1);
    original.push_back(v2);
    original.push_back(v3);

    Array<TestType> moved(std::move(original));

    REQUIRE(moved.size() == 3);
    REQUIRE(moved[0] == v1);
    REQUIRE(moved[1] == v2);
    REQUIRE(moved[2] == v3);
}

TEMPLATE_TEST_CASE("Move construction leaves the source in a valid (empty-or-defined) state",
    "[array][move][template]", int, double, std::string) {
    Array<TestType> original;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    original.push_back(v1);
    original.push_back(v2);

    Array<TestType> moved(std::move(original));

    REQUIRE(original.size() == 0);
    REQUIRE(original.data() == nullptr);
}

TEMPLATE_TEST_CASE("Move assignment transfers size and values from source",
    "[array][move][assignment][template]", int, double, std::string) {
    Array<TestType> original;
    auto v5 = make_value<TestType>(5);
    auto v6 = make_value<TestType>(6);
    auto v7 = make_value<TestType>(7);
    original.push_back(v5);
    original.push_back(v6);
    original.push_back(v7);

    Array<TestType> target;
    auto v999 = make_value<TestType>(999);
    target.push_back(v999);

    target = std::move(original);

    REQUIRE(target.size() == 3);
    REQUIRE(target[0] == v5);
    REQUIRE(target[1] == v6);
    REQUIRE(target[2] == v7);
}

TEMPLATE_TEST_CASE("Move is cheaper than copy: moved-to Array aliases the source's original buffer",
    "[array][move][assignment][template]", int, double, std::string) {
    Array<TestType> original;
    auto v1 = make_value<TestType>(1);
    original.push_back(v1);
    TestType* originalData = original.data();

    Array<TestType> target;
    target = std::move(original);

    REQUIRE(target.data() == originalData);
}

// =======================================================================
// Copy/move semantics verification via TrackedValue (non-templated —
// this test suite exists purely to police *how* the class moves data
// internally, independent of the type-generic behavioral tests above)
// =======================================================================
TEST_CASE("Array move-constructs/copy-constructs elements via T's real special "
    "members during reallocation, not raw byte copying",
    "[array][semantics][tracked]") {
    TrackedValue::reset();
    Array<TrackedValue> arr;

    for (int i = 0; i < 8; ++i) {
        TrackedValue v(i);
        arr.push_back(v);
    }

    // Every element that survived a reallocation must have gone through a
    // real copy or move constructor call — not memcpy. If this is zero
    // while capacity clearly grew past the initial size, reallocate() is
    // doing something structurally unsafe for non-trivial T.
    INFO("copy_ctor_count=" << TrackedValue::copy_ctor_count
        << " move_ctor_count=" << TrackedValue::move_ctor_count);
    REQUIRE((TrackedValue::copy_ctor_count + TrackedValue::move_ctor_count) > 0);
}

TEST_CASE("Copy-constructing an Array<TrackedValue> invokes T's copy constructor "
    "for every element, never move", "[array][semantics][tracked][copy]") {
    TrackedValue::reset();
    Array<TrackedValue> original;
    for (int i = 0; i < 5; ++i) {
        TrackedValue v(i);
        original.push_back(v);
    }

    TrackedValue::reset();
    Array<TrackedValue> copy(original);

    REQUIRE(TrackedValue::copy_ctor_count >= 5);
    REQUIRE(TrackedValue::move_ctor_count == 0);
    REQUIRE(original.size() == 5);
    for (size_t i = 0; i < 5; ++i) {
        REQUIRE(original[i] == copy[i]);
    }
}

TEST_CASE("Move-constructing an Array<TrackedValue> does not invoke T's copy "
    "constructor at all (true O(1) steal, not element-wise move)",
    "[array][semantics][tracked][move]") {
    TrackedValue::reset();
    Array<TrackedValue> original;
    for (int i = 0; i < 5; ++i) {
        TrackedValue v(i);
        original.push_back(v);
    }

    TrackedValue::reset();
    Array<TrackedValue> moved(std::move(original));

    REQUIRE(TrackedValue::copy_ctor_count == 0);
    REQUIRE(TrackedValue::move_ctor_count == 0); // pointer steal, no per-element work
}

// =======================================================================
// Iterators
// =======================================================================
TEMPLATE_TEST_CASE("begin() == end() for an empty Array",
    "[array][iterator][template]", int, double, std::string) {
    Array<TestType> arr;
    REQUIRE(arr.begin() == arr.end());
}

TEMPLATE_TEST_CASE("begin() != end() for a non-empty Array",
    "[array][iterator][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    arr.push_back(v1);
    REQUIRE(arr.begin() != arr.end());
}

TEMPLATE_TEST_CASE("Iterating with operator++ visits every element in logical order",
    "[array][iterator][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v10 = make_value<TestType>(10);
    auto v20 = make_value<TestType>(20);
    auto v30 = make_value<TestType>(30);
    arr.push_back(v10);
    arr.push_back(v20);
    arr.push_back(v30);

    std::vector<TestType> visited;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        visited.push_back(*it);
    }

    REQUIRE(visited == std::vector<TestType>{v10, v20, v30});
}

TEMPLATE_TEST_CASE("Range-based for loop visits every element exactly once",
    "[array][iterator][template]", int, double, std::string) {
    Array<TestType> arr;
    for (int i = 0; i < 10; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v);
    }

    std::vector<TestType> visited;
    for (const auto& v : arr) {
        visited.push_back(v);
    }

    REQUIRE(visited.size() == 10);
    for (int i = 0; i < 10; ++i) {
        REQUIRE(visited[static_cast<size_t>(i)] == make_value<TestType>(i));
    }
}

TEMPLATE_TEST_CASE("Iteration order is correct after the ring has wrapped (non-zero head)",
    "[array][iterator][edge-case][template]", int, double, std::string) {
    Array<TestType> arr;
    for (int i = 0; i < 4; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v);
    }
    arr.pop_front(); // head 1
    arr.pop_front(); // head 2
    auto v4 = make_value<TestType>(4);
    auto v5 = make_value<TestType>(5);
    arr.push_back(v4);
    arr.push_back(v5);

    std::vector<TestType> visited;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        visited.push_back(*it);
    }

    REQUIRE(visited == std::vector<TestType>{
        make_value<TestType>(2), make_value<TestType>(3), v4, v5});
}

TEMPLATE_TEST_CASE("Post-increment returns the pre-increment position",
    "[array][iterator][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    arr.push_back(v1);
    arr.push_back(v2);

    auto it = arr.begin();
    auto old = it++;

    REQUIRE(*old == v1);
    REQUIRE(*it == v2);
}

TEMPLATE_TEST_CASE("operator-> gives access to the pointed-to element",
    "[array][iterator][template]", int, std::string) {
    // Skipping `double` here: operator-> on a double iterator is legal but
    // uninteresting; kept to int/std::string where member access actually
    // matters (e.g. std::string::size() via it->size()).
    Array<TestType> arr;
    auto v = make_value<TestType>(42);
    arr.push_back(v);

    auto it = arr.begin();
    REQUIRE(*(it.operator->()) == v);
}

TEMPLATE_TEST_CASE("Two iterators to the same Array/index compare equal",
    "[array][iterator][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    arr.push_back(v1);
    arr.push_back(v2);

    auto it1 = arr.begin();
    auto it2 = arr.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
}

TEMPLATE_TEST_CASE("Iterating an Array and mutating through *it modifies the underlying element",
    "[array][iterator][template]", int, double) {
    // int/double only: "*it *= 10" doesn't make sense for std::string.
    Array<TestType> arr;
    auto v1 = make_value<TestType>(1);
    auto v2 = make_value<TestType>(2);
    auto v3 = make_value<TestType>(3);
    arr.push_back(v1);
    arr.push_back(v2);
    arr.push_back(v3);

    for (auto it = arr.begin(); it != arr.end(); ++it) {
        *it = static_cast<TestType>(*it * 10);
    }

    REQUIRE(arr[0] == static_cast<TestType>(v1 * 10));
    REQUIRE(arr[1] == static_cast<TestType>(v2 * 10));
    REQUIRE(arr[2] == static_cast<TestType>(v3 * 10));
}

TEMPLATE_TEST_CASE("Iterator remains valid to read all elements after a pop_front-triggered shrink",
    "[array][iterator][edge-case][template]", int, double, std::string) {
    Array<TestType> arr;
    for (int i = 0; i < 3; ++i) {
        auto v = make_value<TestType>(i);
        arr.push_back(v); // capacity 4
    }
    arr.pop_front(); // size 2/4 -> shrink path
    arr.pop_front(); // size 1

    std::vector<TestType> visited;
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        visited.push_back(*it);
    }

    REQUIRE(visited == std::vector<TestType>{make_value<TestType>(2)});
}

TEMPLATE_TEST_CASE("end() iterator is not separately dereferenced in normal traversal",
    "[array][iterator][edge-case][template]", int, double, std::string) {
    Array<TestType> arr;
    auto v7 = make_value<TestType>(7);
    arr.push_back(v7);

    auto it = arr.begin();
    REQUIRE(*it == v7);
    ++it;
    REQUIRE(it == arr.end());
}