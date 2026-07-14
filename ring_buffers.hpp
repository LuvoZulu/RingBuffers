#ifndef RING_BUFFERS_H
#define RING_BUFFERS_H


namespace gabs {

	class Array {
	public:
		Array() {
		}
		Array(const Array& obj) {}
		~Array() {}

		size_t size() const { return size_; }
		size_t capacity() const { return capacity_; }
		void push_back(int val) {}
		void pop_front() {}

	private:
		int* data_ = nullptr;
		size_t size_;
		size_t capacity_;
	};
	
}



#endif // RING_BUFFERS_H