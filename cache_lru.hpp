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

#ifndef CONCURRENT_CACHE_LRU_HPP
#define CONCURRENT_CACHE_LRU_HPP

#include <mutex>
#include <atomic>
#include <cstdlib>
#include <bit>
#include <cinttypes>

#include "mpsc_stack.hpp"

namespace concurrent {
	namespace mpmc {
		template<typename K, typename V, typename ID_TYPE>
		class cache_lru {
		public:
			
			using Key = K;
			using Value = V;
			using Index = ID_TYPE;
			
			const static inline Index BUCKET_SIZE = 64;
			const static inline Index BUCKETS_IN_LEVEL = 64;
			
			
			
			struct element_t {
				Key key;
				Value value;
				Index next;
				Index bucket_position;
				Index flags;	// 0 - empty, 1 - loaded, 2 - dirty, 4 - locked
			};
			
			struct bucket_t {
				struct inner_bucket_t {
					Index elements[BUCKET_SIZE];
				};
				inner_bucket_t buckets[BUCKET_SIZE];
			};
			
			
			
			cache_lru(Index max_elements, Index buckets_of_buckets_count) {
				
				
				
				this->elements_count_max = max_elements;
				this->elements = new element_t[max_elements];
				for(Index i=0; i<max_elements; ++i) {
					elements[i].next = 0;
					elements[i].prev = 0;
					elements[i].bucket_position = 
				}
			}
			
			
		private:
			
			bucket_t* buckets;
			Index buckets_count;
			
			element_t* elements; 
			Index elements_count_max;
			
		};
	}
}

#endif

