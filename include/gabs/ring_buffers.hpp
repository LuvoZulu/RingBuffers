/**
 * @file        ring_buffers.hpp
 * @author      Luvo Zulu
 * @date        2026-07-18
 * @version     1.0.0
 *
 * @brief       A flexible and efficient collection of ring (circular) buffer
 *              implementations in C++.
 *
 * This is the main header of the GABS Ring Buffers library.
 * Include this file to use all containers and utilities.
 *
 * @copyright   (c) Luvo Zulu 2026
 * 
 * 
 * @TODO : Implement testing for linked list
 */

#ifndef RING_BUFFERS_H
#define RING_BUFFERS_H


// ======= Containers =======

#include "containers/array.hpp" // gabs::containers::Array

namespace gabs {

	using Array = gabs::containers::Array;

} // namespace gabs

#endif // RING_BUFFERS_H