/* @file      array.hpp
* @author    Luvo Zulu
* @date      2026 - 07 - 18
* @version   1.0.1
*
* @brief      Project versioning file. This is updated on new releases.
* @license    MIT LICENSE
* @copyright(c) Luvo Zulu 2026
*
*/
#ifndef VERSION_H
#define VERSION_H

#define GABS_VERSION_MAJOR 1
#define GABS_VERSION_MINOR 0
#define GABS_VERSION_PATCH 1

#define GABS_VERSION "1.0.1"


namespace gabs {
	inline constexpr int version_major = GABS_VERSION_MAJOR;
	inline constexpr int version_minor = GABS_VERSION_MINOR;
	inline constexpr int version_patch = GABS_VERSION_PATCH;

	inline const char* version() noexcept {
		return GABS_VERSION;
	}
}




#endif // !VERSION_H