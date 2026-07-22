/**
 * @file      array_impl.hpp
 * @brief     Full implementation of gabs::containers::Array
 */

#ifndef ARRAY_IMPL_H
#define ARRAY_IMPL_H

#include "array.hpp"
#include <algorithm>
#include <utility>
#include <cstring>

namespace gabs::containers {


    template<gabs::concepts::RingBufferCompatible T>
    Array<T>::Array(std::size_t initial_capacity) {
        if (initial_capacity > 0) {
            reallocate(initial_capacity);
        }
    }

    template<gabs::concepts::RingBufferCompatible T>
    Array<T>::Array(const Array& other)
        : head_(other.head_)
        , tail_(other.tail_)
        , size_(other.size_)
        , capacity_(other.capacity_)
    {
        if (capacity_ == 0) return;

        data_ = new T[capacity_];

        for (std::size_t i = 0; i < size_; ++i) {
            std::size_t pos = (head_ + i) % capacity_;
            data_[pos] = other.data_[pos];
        }
    }

    template<gabs::concepts::RingBufferCompatible T>
    Array<T>::Array(Array&& other) noexcept
        : data_(other.data_)
        , head_(other.head_)
        , tail_(other.tail_)
        , size_(other.size_)
        , capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.head_ = 0;
        other.tail_ = 0;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    template<gabs::concepts::RingBufferCompatible T>
    Array<T>::~Array() {
        delete[] data_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    Array<T>& Array<T>::operator=(const Array& other) {
        if (this == &other) return *this;

        delete[] data_;
        data_ = nullptr;

        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        if (capacity_ > 0) {
            data_ = new T[capacity_];
            for (std::size_t i = 0; i < size_; ++i) {
                std::size_t pos = (head_ + i) % capacity_;
                data_[pos] = other.data_[pos];
            }
        }
        return *this;
    }

    template<gabs::concepts::RingBufferCompatible T>
    Array<T>& Array<T>::operator=(Array&& other) noexcept {
        if (this == &other) return *this;

        delete[] data_;

        data_ = other.data_;
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        other.data_ = nullptr;
        other.head_ = 0;
        other.tail_ = 0;
        other.size_ = 0;
        other.capacity_ = 0;

        return *this;
    }

    template<gabs::concepts::RingBufferCompatible T>
    T& Array<T>::operator[](std::size_t idx) {
        if (idx >= size_) throw std::out_of_range("Array index out of range");
        return data_[(head_ + idx) % capacity_];
    }

    template<gabs::concepts::RingBufferCompatible T>
    const T& Array<T>::operator[](std::size_t idx) const {
        if (idx >= size_) throw std::out_of_range("Array index out of range");
        return data_[(head_ + idx) % capacity_];
    }

    template<gabs::concepts::RingBufferCompatible T>
    T& Array<T>::at(std::size_t idx) { return (*this)[idx]; }

    template<gabs::concepts::RingBufferCompatible T>
    const T& Array<T>::at(std::size_t idx) const { return (*this)[idx]; }

    template<gabs::concepts::RingBufferCompatible T>
    T& Array<T>::front() {
        if (empty()) throw std::out_of_range("front() on empty Array");
        return data_[head_];
    }

    template<gabs::concepts::RingBufferCompatible T>
    const T& Array<T>::front() const {
        if (empty()) throw std::out_of_range("front() on empty Array");
        return data_[head_];
    }

    template<gabs::concepts::RingBufferCompatible T>
    T& Array<T>::back() {
        if (empty()) throw std::out_of_range("back() on empty Array");
        return data_[(tail_ + capacity_ - 1) % capacity_];
    }

    template<gabs::concepts::RingBufferCompatible T>
    const T& Array<T>::back() const {
        if (empty()) throw std::out_of_range("back() on empty Array");
        return data_[(tail_ + capacity_ - 1) % capacity_];
    }


    template<gabs::concepts::RingBufferCompatible T>
    typename Array<T>::iterator Array<T>::begin() noexcept {
        return iterator(this, 0);
    }

    template<gabs::concepts::RingBufferCompatible T>
    typename Array<T>::iterator Array<T>::end() noexcept {
        return iterator(this, size_);
    }

    template<gabs::concepts::RingBufferCompatible T>
    typename Array<T>::iterator Array<T>::rbegin() noexcept {
        return iterator(this, size_ - 1);
    }

    template<gabs::concepts::RingBufferCompatible T>
    typename Array<T>::iterator Array<T>::rend() noexcept {
        return iterator(this, static_cast<std::size_t>(-1));
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::push_back(const T& val) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[tail_] = val;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::push_back(T&& val) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[tail_] = std::move(val);
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::push_front(const T& val) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        head_ = (head_ + capacity_ - 1) % capacity_;
        data_[head_] = val;
        ++size_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::push_front(T&& val) {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        head_ = (head_ + capacity_ - 1) % capacity_;
        data_[head_] = std::move(val);
        ++size_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::pop_front() {
        if (size_ == 0) return;
        head_ = (head_ + 1) % capacity_;
        --size_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::pop_back() {
        if (size_ == 0) return;
        tail_ = (tail_ + capacity_ - 1) % capacity_;
        --size_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::clear() noexcept {
        size_ = 0;
        head_ = 0;
        tail_ = 0;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::resize(std::size_t new_size) {
        if (new_size > capacity_) {
            reserve(new_size);
        }
        size_ = new_size;
        tail_ = (head_ + size_) % capacity_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::resize(std::size_t new_size, const T& value) {
        if (new_size > capacity_) {
            reserve(new_size);
        }

        while (size_ < new_size) {
            push_back(value);
        }
        size_ = new_size;
        tail_ = (head_ + size_) % capacity_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_) return;
        reallocate(new_capacity);
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::shrink_to_fit() {
        if (size_ == capacity_) return;
        reallocate(size_);
    }

    template<gabs::concepts::RingBufferCompatible T>
    void Array<T>::reallocate(std::size_t new_capacity) {
        if (new_capacity == 0) new_capacity = 1;

        T* new_data = new T[new_capacity];

        for (std::size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move((*this)[i]);
        }

        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
        head_ = 0;
        tail_ = size_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    typename Array<T>::iterator Array<T>::iterator::operator++(int) {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    template<gabs::concepts::RingBufferCompatible T>
    typename Array<T>::iterator Array<T>::iterator::operator--(int) {
        iterator tmp = *this;
        --(*this);
        return tmp;
    }

    template<gabs::concepts::RingBufferCompatible T>
    bool Array<T>::iterator::operator==(const iterator& other) const {
        return buffer_ == other.buffer_ && idx_ == other.idx_;
    }

    template<gabs::concepts::RingBufferCompatible T>
    bool Array<T>::iterator::operator!=(const iterator& other) const {
        return !(*this == other);
    }

} // namespace gabs::containers

#endif // ARRAY_IMPL_H