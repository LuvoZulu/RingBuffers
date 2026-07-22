/**
* @file      array.hpp
* @author    Luvo Zulu
* @date      2026 - 07 - 18
* @version   1.0.1
*
*@brief      A flexible and efficient collection of ring array buffer implementations.
*            This will include both static and dynamic arrays
*@license    MIT LICENSE
*@copyright(c) Luvo Zulu 2026
* 
*/

#ifndef ARRAY_H
#define ARRAY_H


#include <algorithm> // std::swap
#include <stdexcept> // std::out_of_range

// ====== Library Configuration =======
#include "gabs/version.hpp"
#include "gabs/config.hpp"
#include "gabs/concepts.hpp"
#include "gabs/type_traits.hpp"

namespace gabs::containers {

    
    template<typename T>
    class Array {
    public:
        class iterator;

        Array() : head_(0), tail_(0), size_(0), capacity_(0), data_(nullptr) {}

        Array(const Array& other);
        Array(Array&& other) noexcept;
        ~Array();

        Array& operator=(const Array& other);
        Array& operator=(Array&& other) noexcept;

        T& operator[](size_t idx);
        const T& operator[](size_t idx) const;

        iterator begin();
        iterator end();

        void push_back(T& val);
        void pop_front();

        T* data() const { return data_; }
        size_t size() const { return size_; }
        size_t capacity() const { return capacity_; }
        size_t head() const { return head_; }
        size_t tail() const { return tail_; }

    private:
        void reallocate(size_t new_size);

        T* data_;
        size_t head_;
        size_t tail_;
        size_t size_;
        size_t capacity_;
    };

    Array(const char*)->Array<char>;
}

#endif // !ARRAY_H

#include "array_impl.hpp"