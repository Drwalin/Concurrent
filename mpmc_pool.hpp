/*
 *  Concurrent primitive data structures.
 *  Copyright (C) 2021 Marek Zalewski aka Drwalin
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

#ifndef CONCURRENT_MPMC_POOL_HPP
#define CONCURRENT_MPMC_POOL_HPP

#include <atomic>
#include <cstdlib>
#include <bit>
#include <cinttypes>
#include <mutex>

#include "node.hpp"
#include "node_stack.hpp"
#include "mpsc_stack.hpp"

namespace concurrent {
	namespace mpmc {
		template<typename T>
		class pool {
		public:
			
			pool() = default;
			pool(pool&&o) = delete;
			pool(const pool&) = delete;
			~pool() = default;
			
			inline pool& operator=(pool&&) = default;
			inline pool& operator=(const pool&) = delete;
			
			template<typename... _args>
			inline void reserve(size_t n, _args... args) {
				if(n == 0)
					return;
				nonconcurrent::node_stack<T> tmp;
				for(; n; --n)
					tmp.push(new T, args...);
				approximate_pool_size += n;
			}
			
			template<typename... _args_for_new_objecet>
			inline T* aquire(_args_for_new_objecet... args) {
				{
					std::lock_guard<std::mutex> lock(mutex);
					T* ret = stack.pop();
					if(ret) {
						--approximate_pool_size;
						return ret;
					}
				}
				return new T(args...);
			}
			
			inline void release(T* ptr) {
				stack.push(ptr);
				++approximate_pool_size;
			}
			
			inline size_t get_approximate_allocated_objects() const {
				return approximate_pool_size;
			}
			
		private:
			
			mpsc::stack<T> stack;
			std::mutex mutex;
			std::atomic<size_t> approximate_pool_size;
		};
	}
}

#endif

