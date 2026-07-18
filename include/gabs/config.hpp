/* @file      array.hpp
* @author    Luvo Zulu
* @date      2026 - 07 - 18
* @version   1.0.1
*
*@brief      Project configuration file. This file serves as a single source of truth for common functionality.
* @license    MIT LICENSE
* @copyright(c) Luvo Zulu 2026
*
*/

#ifndef CONFIG_H
#define CONFIG_H

#ifndef GABS_NO_EXCEPTIONS
#define GABS_NO_EXCEPTIONS 0
#endif // !GABS_NO_EXCEPTIONS

#ifndef GABS_ENABLE_ASSERTIONS
#define GABS_ENABLE_ASSERTIONS 1
#endif // !GABS_ENABLE_ASSERTIONS

#ifndef GABS_DEFAULT_GROWTH_FACTOR
#define GABS_DEFAULT_GROWTH_FACTOR 1
#endif // !GABS_DEFAULT_GROWTH_FACTOR

#ifdef _DEBUG
	#define GABS_DEBUG 1
#else 
	#define GABS_DEBUG 0
#endif // _DEBUG


namespace gabs::config {
	inline constexpr bool exceptions_enabled = GABS_NO_EXCEPTIONS == 0;
	inline constexpr bool assertions_enabled = GABS_ENABLE_ASSERTIONS == 1;
	inline constexpr size_t default_growth_factor = GABS_DEFAULT_GROWTH_FACTOR;
}

#endif // !CONFIG_H
