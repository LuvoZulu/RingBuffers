#ifndef RING_BUFFERS_H
#define RING_BUFFERS_H


namespace gabs {

	class Array {
	public:
		Array() {
		}
		Array(const Array& obj) {}
		Array(Array&& obj) {}
		~Array() {}

		int* data() const { return data_; };
		size_t size() const { return size_; }
		size_t capacity() const { return capacity_; }

		void push_back(int val) {

			if (capacity_ == 0) {

			}


		}

		void pop_front() {

		}

	private:

		void reallocate(size_t& new_size) {
			int* new_data = new int[new_size];
			const int ctrl = 1;
			auto index = 0;

			for (auto i{ 0uz }; i < size_; i++) {
				index = (index + ctrl) & (capacity_ - 1);
				*(new_data + index) = data_[index];
			}

			capacity_ = new_size;

			delete data_;
			data_ = new_data; // NOTE: This escapes this scope
		}

		int* data_ = nullptr;
		size_t size_;
		size_t capacity_;
	};
	
}



#endif // RING_BUFFERS_H