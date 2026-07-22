#pragma once
#include <type_traits>


namespace gabs::traits {
	template<typename T>
	struct is_safe_for_ring_buffer : std::bool_constant<std::is_trivially_copyable<T> && 
									 std::is_default_constructible_v<T>> {};
	template<typename T>
	inline constexpr is_safe_for_ring_buffer_v = is_safe_for_ring_buffer<T>::value;

	template<typename T, typename = void>
	struct has_reserve : std::false_type {};

	template<typename T>
	struct has_reserve<T, std::void_t<decltype(std::declval<T>().reserve(0))>>
		: std::true_type {};

	template<typename T>
	inline constexpr bool has_reserve_v = has_reserve<T>::value;


} // gabs::traits