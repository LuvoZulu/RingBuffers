#pragma once


namespace gabs::containers {
    // ====================== Iterator Definition ======================

    template<typename T>
    class Array<T>::iterator {
    public:
        iterator(Array* buffer, std::size_t index)
            : buffer_(buffer), idx_(index) {}

        T& operator*() const {
            return (*buffer_)[idx_];
        }

        T* operator->() const {
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

    template<typename T>
    inline Array<T>::Array(const Array<T>& other)
        : data_(nullptr), head_(other.head_), tail_(other.tail_), size_(other.size_), capacity_(other.capacity_)
        
    {
        if (capacity_ == 0) return;

        data_ = new T[capacity_];

        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other[(head_ + i) & (capacity_ - 1)];
        }
    }

    template<typename T>
    inline Array<T>::Array(Array<T>&& other) noexcept
        : data_(other.data_), head_(other.head_), tail_(other.tail_),
        size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        other.head_ = 0;
        other.tail_ = 0;
    }

    template<typename T>
    inline Array<T>::~Array() {
        delete[] data_;
    }

    template<typename T>
    inline Array<T>& Array<T>::operator=(const Array<T>& other) {
        if (this == &other) return *this;

        delete[] data_;
        data_ = nullptr;

        size_ = other.size_;
        capacity_ = other.capacity_;
        head_ = other.head_;
        tail_ = other.tail_;

        if (capacity_ > 0) {
            data_ = new T[capacity_];
            for (size_t i = 0; i < size_; ++i) {
                data_[i] = other[(head_ + i) & (capacity_ - 1)];
            }
        }
        return *this;
    }

    template<typename T>
    inline Array<T>& Array<T>::operator=(Array<T>&& other) noexcept {
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

    template<typename T>
    inline T& Array<T>::operator[](size_t idx) {
        if (idx < 0 || idx >= size_)
            throw std::out_of_range("Array index out of range");

        return data_[(head_ + idx) & (capacity_ - 1)];
    }
    template <typename T>
    inline const T& Array<T>::operator[](size_t idx) const {
        return data_[(head_ + idx) & (capacity_ - 1)];
    }

    template<typename T>
    inline void Array<T>::push_back(T& val) {
        if (capacity_ == size_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }

        data_[tail_] = val;
        tail_ = (tail_ + 1) & (capacity_ - 1);
        ++size_;
    }

    template<typename T>
    inline void Array<T>::pop_front() {
        if (size_ == 0) return;

        head_ = (head_ + 1) & (capacity_ - 1);
        --size_;

        if (size_ < capacity_ / 2 && capacity_ > 1) {
            reallocate(capacity_ / 2);
        }
    }

    template<typename T>
    inline void Array<T>::reallocate(size_t new_size) {
        T* new_data = new T[new_size];

        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = (*this)[i];
        }

        delete[] data_;
        data_ = new_data;
        capacity_ = new_size;
        head_ = 0;
        tail_ = size_;
    }

    template<typename T>
    inline typename Array<T>::iterator Array<T>::begin() {
        return iterator(this, 0);
    }
    
    template<typename T>
    inline typename Array<T>::iterator Array<T>::end() {
        return iterator(this, size_);
    }
}