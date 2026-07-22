#ifndef GABS_CONCEPTS
#define GABS_CONCEPTS


#include <concepts>
#include <type_traits>


namespace gabs::concepts {

	template<typename T>
	concept TriviallyCopyable = std::is_trivially_copyable<T>;

	template<typename T>
	concept DefaultConstructible = std::default_initializable<T>;

	template<typename T>
	concept RingBufferCompatible = DefaultConstructible<T> && TriviallyCopyable<T>;

	template<typename T>
	concept Iterator = requires(T it) {
		*it;
		++it;
		it++;
		it == it;
	};

	template<typename T>
	concept PointerIterator = std::is_pointer<T> && Iterator<T>;
}




#endif // !GABS_CONCEPTS