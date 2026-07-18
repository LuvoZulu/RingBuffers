/**
 * @file        ring_buffers.hpp
 * @author      Luvo Zulu
 * @date        2026-07-18
 * @version     1.0.0
 *
 * @brief       A flexible and efficient collection of ring (circular) buffer
 *              implementations in C++.
 *
 * @copyright   (c) Luvo Zulu 2026
 */

#ifndef RING_BUFFERS_H
#define RING_BUFFERS_H

#include <algorithm> // std::swap
#include <stdexcept> // std::out_of_range

namespace gabs {

    using type = int;

    class Array {
    public:
        class iterator;

        Array() : head_(0), tail_(0), size_(0), capacity_(0), data_(nullptr) {}

        Array(const Array& other);
        Array(Array&& other) noexcept;
        ~Array();

        Array& operator=(const Array& other);
        Array& operator=(Array&& other) noexcept;

        type& operator[](size_t idx);
        const type& operator[](size_t idx) const;

        iterator begin();
        iterator end();

        void push_back(type val);
        void pop_front();

        type* data() const { return data_; }
        size_t size() const { return size_; }
        size_t capacity() const { return capacity_; }
        size_t head() const { return head_; }
        size_t tail() const { return tail_; }

    private:
        void reallocate(size_t new_size);

        type* data_;
        size_t head_;
        size_t tail_;
        size_t size_;
        size_t capacity_;
    };

    // ====================== Iterator Definition ======================

    class Array::iterator {
    public:
        iterator(Array* buffer, std::size_t index)
            : buffer_(buffer), idx_(index) {}

        type& operator*() const {
            return (*buffer_)[idx_];
        }

        type* operator->() const {
            return &(*buffer_)[idx_];
        }

        iterator& operator++() {
            ++idx_;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const iterator& other) const {
            return buffer_ == other.buffer_ && idx_ == other.idx_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        Array* buffer_ = nullptr;
        std::size_t idx_ = 0;
    };

    // ====================== Array Implementation ======================

    inline Array::Array(const Array& other)
        : size_(other.size_), capacity_(other.capacity_),
        head_(other.head_), tail_(other.tail_), data_(nullptr)
    {
        if (capacity_ == 0) return;

        data_ = new type[capacity_];

        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other[(head_ + i) & (capacity_ - 1)];
        }
    }

    inline Array::Array(Array&& other) noexcept
        : data_(other.data_), head_(other.head_), tail_(other.tail_),
        size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
    }

    inline Array::~Array() {
        delete[] data_;
    }

    inline Array& Array::operator=(const Array& other) {
        if (this == &other) return *this;

        delete[] data_;
        data_ = nullptr;

        size_ = other.size_;
        capacity_ = other.capacity_;
        head_ = other.head_;
        tail_ = other.tail_;

        if (capacity_ > 0) {
            data_ = new type[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other[(head_ + i) & (capacity_ - 1)];
            }
        }
        return *this;
    }

    inline Array& Array::operator=(Array&& other) noexcept {
        if (this == &other) return *this;

        delete[] data_;

        data_ = other.data_;
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;

        return *this;
    }

    inline type& Array::operator[](size_t idx) {
        if (idx < 0 || idx >= size_)
            throw std::out_of_range("Array index out of range");

        return data_[(head_ + idx) & (capacity_ - 1)];
    }

    inline const type& Array::operator[](size_t idx) const {
        return data_[(head_ + idx) & (capacity_ - 1)];
    }

    inline void Array::push_back(type val) {
        if (capacity_ == size_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }

        data_[tail_] = val;
        tail_ = (tail_ + 1) & (capacity_ - 1);
        ++size_;
    }

    inline void Array::pop_front() {
        if (size_ == 0) return;

        head_ = (head_ + 1) & (capacity_ - 1);
        --size_;

        if (size_ < capacity_ / 2 && capacity_ > 1) {
            reallocate(capacity_ / 2);
        }
    }

    inline void Array::reallocate(size_t new_size) {
        type* new_data = new type[new_size];

        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = (*this)[i];
        }

        delete[] data_;
        data_ = new_data;
        capacity_ = new_size;
        head_ = 0;
        tail_ = size_;
    }

    inline Array::iterator Array::begin() {
        return iterator(this, 0);
    }

    inline Array::iterator Array::end() {
        return iterator(this, size_);
    }

} // namespace gabs

#endif // RING_BUFFERS_H