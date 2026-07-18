/**
 * @file        ring_buffers.hpp
 * @author      Luvo Zulu
 * @date        2026-07-14
 * @version     1.0.0
 *
 * @brief       A flexible and efficient collection of ring (circular) buffer
 *              implementations in C++.
 *
 * This is a living project containing various ring buffer implementations
 * (dynamic, static, lock-free ready, etc.). The goal is to provide high-performance,
 * easy-to-use circular buffer solutions for different use cases.
 *
 * Feel free to use and contribute. If you find any performance issues, logical bugs,
 * or have suggestions for improvements, please reach out.
 *
 * @note        Currently focused on dynamic memory allocation with plans to add
 *              compile-time static allocation support.
 *
 * @see         array
 * @todo        - Implement Doubly Linked List based ring buffer
 *              - Add lock-free version for multi-threaded use
 *              - Add static memory allocation option
 *              - Improve iterator support
 *
 * @copyright   (c) Luvo Zulu 2026
 */


#ifndef RING_BUFFERS_H
#define RING_BUFFERS_H

#include <algorithm>  // std::swap

namespace gabs {

	using type = int;

	class array {
	public:
		array() {
			head_ = 0;
			tail_ = 0;
			size_ = 0;
			capacity_ = 0;
		}

		
		array(const array& obj) : size_{ obj.size() }, capacity_{obj.capacity()},
			head_{obj.head()}, tail_{obj.tail()}
		{
			if (data_ == obj.data()) return;

			data_ = new int[capacity_];

			for (auto i{ 0uz }; i < size_; i++) {
				data_[i] = obj[(head_ + i) & (capacity_ - 1)];
			}

		}

		array(array&& obj) {}


		int& operator[](size_t idx) { return data_[(head_ + idx) & (capacity_ - 1)]; }
		const int& operator[](size_t idx) const { return data_[(head_ + idx) & (capacity_ - 1)]; }

		array& operator++() = delete;
		array operator++(int) = delete;

		array& operator=(const array& other)
		{
			if (this == &other)
				return *this;

			int* new_data = new int[other.capacity_];

			for (size_t i = 0; i < other.size_; ++i)
				new_data[i] = other[i];

			delete[] data_;

			data_ = new_data;
			size_ = other.size_;
			capacity_ = other.capacity_;
			head_ = 0;
			tail_ = size_;

			return *this;
		}

		array& operator=(array& other) {
			std::swap(*this, other);
			return *this;
		}

		array& operator=(array&& other) {
			std::swap(*this, other);
			return *this;
		}

		~array() {}

		type* data() const { return data_; };
		size_t size() const { return size_; }
		size_t capacity() const { return capacity_; }
		size_t tail() const { return tail_; }
		size_t head() const { return head_; }

		// TODO: These two are currently incorrect, use the logic of circular arrays here
		type* begin() { return data_; }
		type* end() { return ; }


		/*
		* THIS CURRENT VERSION IS THE DYNAMIC MEMORY ALLOCATION
		* NEXT FEATURE: flag to ensure we choose between dynamic and static memory allocation
		*/
		void push_back(type val) {

			if (capacity_ == 0) {
				size_t new_size = 1;
				reallocate(new_size);
			
			}
			else if (capacity_ == size_) {
				size_t new_size = capacity_ * 2;
				reallocate(new_size);
			}

			data_[tail_] = val;
			tail_ = (tail_ + 1) & (capacity_ - 1);
			size_++;
		}

		void pop_front() {

			if (size_ < 0) return;

			head_ = (head_ + 1) & (capacity_ - 1);
			size_--;

			if (size_ < 0.5 * capacity_) {
				auto new_size = capacity_ / 2;
				reallocate(new_size);
			}
		}

		class iterator;
	private:

		void reallocate(size_t& new_size) {
			type* new_data = new type[new_size];

			for (auto i{ 0uz }; i < size_; i++) {
				*(new_data + i) = data_[(head_ + i) & (capacity_ - 1)];
			}

			capacity_ = new_size;

			delete [] data_;
			data_ = new_data; // NOTE: This escapes this scope
		}

		type* data_ = nullptr;
		size_t head_;
		size_t tail_;
		size_t size_;
		size_t capacity_;
	};

	/*
	* @ brief     A simple forward iterator class to assist the functionality for arrays
	* @ params    We need both the head and tail pointers of the objects we are iterating
	* @ throws    Undefined behaviour, No Language error is thrown
	*/
	class array::iterator
	{
	public:

		iterator(array* buffer, std::size_t index)
			: buffer_(buffer),
			idx(index)
		{}

		type& operator*() const
		{
			return (*buffer_)[idx];
		}

		type* operator->() const
		{
			return &(*buffer_)[idx];
		}

		iterator& operator++()
		{
			++idx;
			return *this;
		}

		iterator operator++(int)
		{
			iterator temp = *this;
			++(*this);
			return temp;
		}

		bool operator==(const iterator& other) const
		{
			return buffer_ == other.buffer_
				&& idx == other.idx;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

	private:
		array* buffer_ = nullptr;
		std::size_t idx = 0;
	};

}



#endif // RING_BUFFERS_H