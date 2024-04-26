/*
 *  Concurrent primitive data structures.
 *  Copyright (C) 2022 Marek Zalewski aka Drwalin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CONCURRENT_MPMC_TB_POOL_HPP
#define CONCURRENT_MPMC_TB_POOL_HPP

#include <atomic>
#include <cstdlib>
#include <bit>
#include <cinttypes>
#include <mutex>
#include <vector>

#include "node.hpp"
#include "node_stack.hpp"
#include "mpsc_stack.hpp"

namespace concurrent {
	namespace mpmc {
		namespace tb {
		template<typename T, size_t objects_per_buffer_limit=128,
			size_t full_buffers_limit=32, size_t empty_buffers_limit=32>
			class pool {
			public:
				
				class buffer : concurrent::node<buffer> {
				public:
					buffer(buffer) = delete;
					buffer(buffer&) = delete;
					buffer(buffer&&) = delete;
					buffer(const buffer&) = delete;
					buffer& operator=(buffer) = delete;
					buffer& operator=(buffer&) = delete;
					buffer& operator=(buffer&&) = delete;
					buffer& operator=(const buffer&) = delete;
					buffer() {
						size = 0;
					}
					template<typename... _args_for_new_objecet>
					buffer(size_t preallocate, _args_for_new_objecet... args) {
						size = preallocate;
						if(size > capacity)
							size = capacity;
						for(size_t i=0; i<size; ++i)
							objects[i] = new T(args);
					}
					~buffer() {
						for(size_t i=0; i<size; ++i) {
							delete objects[i];
						}
						size = 0;
					}
					
					T* pop() {
						if(size) {
							size--;
							return objects[size];
						}
						return NULL;
					}
					bool push(T* object) {
						if(size < capacity) {
							objects[size] = object;
							++size;
							return true;
						}
						return false;
					}
					
					const static inline size_t capacity
						= objects_per_buffer_limit;
					T* objects[capacity];
					size_t size;
				};
				
				static mpmc::pool<buffer>& get_full_buffer_pool() {
					static mpmc::pool<buffer> _pool;
					return _pool;
				}
				
				static mpmc::pool<buffer>& get_empty_buffer_pool() {
					static mpmc::pool<buffer> _pool;
					return _pool;
				}
				
				class local_pool {
				public:
					
					local_pool() {
						current = NULL;
					}
					~local_pool() {
						release_buffer();
					}
					
					void release_buffer() {
						if(current) {
							if(current->size < buffer::capacity) {
								auto& p = get_empty_buffer_pool();
								if(p.get_approximate_allocated_objects() >=
										empty_buffers_limit) {
									delete current;
								} else {
									p.release(current);
								}
							} else {
								auto& p = get_full_buffer_pool();
								if(p.get_approximate_allocated_objects() >=
										full_buffers_limit) {
									delete current;
								} else {
									p.release(current);
								}
							}
							current = NULL;
						}
					}
					
					void acquire_empty_buffer() {
						release_buffer();
						current = get_empty_buffer_pool().acquire();
					}
					
					template<typename... _args_for_new_objecet>
					void acquire_full_buffer(_args_for_new_objecet... args) {
						release_buffer();
						current = get_full_buffer_pool()
							.acquire(buffer::capaciy, args...);
					}
					
					
					template<typename... _args_for_new_objecet>
					T* acquire(_args_for_new_objecet... args) {
						T* ptr = NULL;
						if(current == NULL)
							acquire_full_buffer(args...):
						else if(current->size == 0)
							acquire_full_buffer(args...):
						
						if(curren) {
							ptr = current->pop(args...);
						} else {
							// TODO: check - should not happen
						}
						
						if(ptr)
							return ptr;
						else
							return new T(args...);
					}
					
					void release(T* ptr) {
						if(current == NULL)
							acquire_empty_buffer();
						else if(current->size >= buffer::capacity)
							acquire_empty_buffer();
						
						if(current) {
							if(current->push(ptr) == false) {
								// TODO: check - should not happen
								delete ptr;
							}
						} else {
							// TODO: check - should not happen
						}
					}
					
					buffer* current;
				};

				
				static local_pool& get_thread_local_pool() {
					static thread_local local_pool thread_local_pool;
					return thread_local_pool;
				}
				
				
				template<typename... _args_for_new_objecet>
				static T* acquire(_args_for_new_objecet... args) {
					return get_thread_local_pool().acquire(args...);
				}

				static void release(T* ptr) {
					get_thread_local_pool().release(ptr);
				}
			}
		}
	}
}

#endif

