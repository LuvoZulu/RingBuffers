#ifndef RING_BUFFERS_H
#define RING_BUFFERS_H

#include <algorithm>  // std::swap

namespace gabs {

	class Array {
	public:
		Array() {
			head_ = 0;
			tail_ = 0;
			size_ = 0;
			capacity_ = 0;
		}

		
		Array(const Array& obj) : size_{ obj.size() }, capacity_{obj.capacity()},
			head_{obj.head()}, tail_{obj.tail()}
		{
			if (data_ == obj.data()) return;

			data_ = new int[capacity_];

			for (auto i{ 0uz }; i < size_; i++) {
				data_[i] = obj[(head_ + i) & (capacity_ - 1)];
			}

		}

		Array(Array&& obj) {}


		int& operator[](size_t idx) { return data_[(head_ + idx) & (capacity_ - 1)]; }
		const int& operator[](size_t idx) const { return data_[(head_ + idx) & (capacity_ - 1)]; }

		Array& operator++() = delete;
		Array operator++(int) = delete;

		Array& operator=(const Array& other)
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

		Array& operator=(Array& other) {
			std::swap(*this, other);
			return *this;
		}

		Array& operator=(Array&& other) {
			std::swap(*this, other);
			return *this;
		}

		~Array() {}

		int* data() const { return data_; };
		size_t size() const { return size_; }
		size_t capacity() const { return capacity_; }
		size_t tail() const { return tail_; }
		size_t head() const { return head_; }

		// TODO: These two are currently incorrect, use the logic of circular arrays here
		int* begin() { return data_; }
		int* end() { return ; }


		/*
		* THIS CURRENT VERSION IS THE DYNAMIC MEMORY ALLOCATION
		* NEXT FEATURE: flag to ensure we choose between dynamic and static memory allocation
		*/
		void push_back(int val) {

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

	private:

		void reallocate(size_t& new_size) {
			int* new_data = new int[new_size];

			for (auto i{ 0uz }; i < size_; i++) {
				*(new_data + i) = data_[(head_ + i) & (capacity_ - 1)];
			}

			capacity_ = new_size;

			delete data_;
			data_ = new_data; // NOTE: This escapes this scope
		}

		int* data_ = nullptr;
		size_t head_;
		size_t tail_;
		size_t size_;
		size_t capacity_;
	};
	
}



#endif // RING_BUFFERS_H