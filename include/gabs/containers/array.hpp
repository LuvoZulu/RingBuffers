#ifndef ARRAY_H
#define ARRAY_H

#include <stdexcept>
#include <cstddef>

// ====== Library Configuration =======
#include "gabs/version.hpp"
#include "gabs/config.hpp"
#include "gabs/concepts.hpp"
#include "gabs/type_traits.hpp"

namespace gabs::containers {

    template<gabs::concepts::RingBufferCompatible T>
    class Array {
    public:
        class iterator {
        public:
            using value_type = T;
            using pointer = T*;
            using reference = T&;

            iterator(Array* buff, std::size_t idx) : buffer_{ buff }, idx_{ idx } {}

            reference operator*() const { return (*buffer_)[idx_]; }
            pointer operator->() const { return &(*buffer_)[idx_]; }

            iterator& operator++() { ++idx_; return *this; }
            iterator operator++(int);
            iterator& operator--() { --idx_; return *this; }
            iterator operator--(int);

            bool operator==(const iterator& other) const;
            bool operator!=(const iterator& other) const;

        private:
            Array* buffer_ = nullptr;
            std::size_t idx_ = 0;
        };

        Array() noexcept = default;
        explicit Array(std::size_t capacity);
        Array(const Array& other);
        Array(Array&& other) noexcept;
        ~Array();

        Array& operator=(const Array& other);
        Array& operator=(Array&& other) noexcept;

        T& operator[](std::size_t idx);
        const T& operator[](std::size_t idx) const;

        T& at(std::size_t idx);
        const T& at(std::size_t idx) const;

        T& front();
        const T& front() const;
        T& back();
        const T& back() const;

        iterator begin() noexcept;
        iterator end() noexcept;
        iterator rbegin() noexcept;
        iterator rend() noexcept;

        std::size_t size() const noexcept { return size_; }
        std::size_t capacity() const noexcept { return capacity_; }
        bool empty() const noexcept { return size_ == 0; }
        void reserve(std::size_t new_capacity);
        void shrink_to_fit();

        void push_back(const T& val);
        void push_back(T&& val);
        void push_front(const T& val);
        void push_front(T&& val);

        void pop_front();
        void pop_back();

        void clear() noexcept;
        void resize(std::size_t new_size);
        void resize(std::size_t new_size, const T& value);

        T* data() noexcept { return data_; }
        const T* data() const noexcept { return data_; }

    private:
        void reallocate(std::size_t new_capacity);

        T* data_ = nullptr;
        std::size_t head_ = 0;
        std::size_t tail_ = 0;
        std::size_t size_ = 0;
        std::size_t capacity_ = 0;
    };

    Array(const char*)->Array<char>;

} // namespace gabs::containers

#include "array_impl.hpp"

#endif // ARRAY_H