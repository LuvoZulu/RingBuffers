#ifndef RING_BUFFERS_H
#define RING_BUFFERS_H

namespace gabs {

	class Array {
	public:
		Array() {
			head_ = 0;
			tail_ = 0;
			size_ = 0;
			capacity_ = 0;
		}
		Array(const Array& obj) {}
		Array(Array&& obj) {}
		~Array() {}

		int* data() const { return data_; };
		size_t size() const { return size_; }
		size_t capacity() const { return capacity_; }


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

			if (size_ <= 0.5 * capacity_) {
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