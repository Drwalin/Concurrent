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

#ifndef CONCURRENT_HASHMAP_HPP
#define CONCURRENT_HASHMAP_HPP

#include <mutex>
#include <atomic>
#include <cstdlib>
#include <bit>
#include <cinttypes>

#include "mpsc_stack.hpp"
#include "mutex_aligned.hpp"
#include "hash.hpp"

namespace concurrent {
	namespace mpmc {
		template<typename K, typename V>
		struct kp {
			struct default_hashmap_node {
				K key;
				V value;
				struct default_hashmap_node* next;
			};
			
			template<typename T>
			struct default_hashmap_allocator {
				T* allocate() {
					return new T();
				}
				void free(T* ptr) {
					delete ptr;
				}
			};
			
			/*
			 * MUTEX_COUNT - must be power of 2
			 */
			template<uint64_t(*HASH)(K)
				=&concurrent::default_hash::hash<K>,
				int MUTEX_COUNT=1024*16, typename NODE=default_hashmap_node,
				typename ALLOCATOR=default_hashmap_allocator<NODE>>
			class hashmap {
			public:
				
// 				static_assert (__builtin_popcount(MUTEX_COUNT) <= 1,
// 						"MUTEX_COUNT must be power of 2");
				
				using Key = K;
				using Value = V;
				using Node = NODE;
				using Allocator = ALLOCATOR;
				inline uint64_t Hash(K key) {return HASH(key);}
				
				/*
				 * max_elements - should be a prime number. Number of elements
				 *                cannot surplus this value. Must be less than
				 *                or equal to Index::MAX_VALUE-2
				 */
				hashmap(uint64_t max_elements, Allocator* allocator=NULL) {
					this->allocator = allocator;
					this->size = max_elements;
					this->buckets = new Node*[this->size];
					for(uint64_t i=0; i<size; ++i) {
						this->buckets[i] = NULL;
					}
				}
				~hashmap() {
					for(uint64_t i=0; i<size; ++i) {
						Node* node = buckets[i];
						while(node) {
							Node* next = node->next;
							allocator->free(node);
							node = next;
						}
					}
					delete[] buckets;
				}
				
				inline bool try_put_new(const Key& key, Value& value,
						Allocator* allocator) {
					Node* node = allocator->allocate();
					if(node == NULL)
						return false;
					node->key = key;
					node->value = value;
					const uint64_t hash = Hash(key);
					const uint64_t id = hash % size;
					const uint64_t mutex_id = id % (MUTEX_COUNT);
					lock_write(mutex_id);
					node->next = buckets[id];
					buckets[id] = node;
					unlock_write(mutex_id);
					return true;
				}
				
				inline bool try_put_new(const Key& key, Value& value) {
					Node* node = allocator->allocate();
					if(node == NULL)
						return false;
					node->key = key;
					node->value = value;
					const uint64_t hash = Hash(key);
					const uint64_t id = hash % size;
					const uint64_t mutex_id = id % (MUTEX_COUNT);
					lock_write(mutex_id);
					node->next = buckets[id];
					buckets[id] = node;
					unlock_write(mutex_id);
					return true;
				}
				
				inline Node* find(const Key& key, uint64_t id) {
					Node* node = buckets[id];
					while(node) {
						if(node->key == key)
							return node;
						node = node->next;
					}
					return node;
				}
				
				inline bool try_set(const Key& key, Value& value) {
					const uint64_t hash = Hash(key);
					const uint64_t id = hash % size;
					const uint64_t mutex_id = id % (MUTEX_COUNT);
					lock_write(mutex_id);
					Node* node = find(key, id);
					if(node) {
						node->value = value;
						unlock_write(mutex_id);
						return true;
					}
					unlock_write(mutex_id);
					node = allocator->allocate();
					if(node == NULL) {
						return false;
					}
					node->key = key;
					node->value = value;
					lock_write(mutex_id);
					node->next = buckets[id];
					buckets[id] = node;
					unlock_write(mutex_id);
					return true;
				}
				
				inline bool try_get(const Key& key, Value& value) {
					const uint64_t hash = Hash(key);
					const uint64_t id = hash % size;
					const uint64_t mutex_id = id & (MUTEX_COUNT-1);
					lock_read(mutex_id);
					Node* node = find(key, id);
					if(node == NULL) {
						unlock_read(mutex_id);
						return false;
					}
					value = node->value;
					unlock_read(mutex_id);
					return true;
				}
				
				inline void remove(const Key& key, Allocator* allocator) {
					const uint64_t hash = Hash(key);
					const uint64_t id = hash % size;
					const uint64_t mutex_id = id % (MUTEX_COUNT);
					lock_write(mutex_id);
					Node* prev = NULL;
					Node* node = buckets[id];
					while(node) {
						if(node->key == key)
							break;
						prev = node;
						node = node->next;
					}
					if(node == NULL) {
						unlock_write(mutex_id);
						return;
					}
					if(prev)
						prev->next = node->next;
					else
						buckets[id] = node->next;
					unlock_write(mutex_id);
					allocator->free(node);
					return;
				}
				
				inline void remove(const Key& key) {
					const uint64_t hash = Hash(key);
					const uint64_t id = hash % size;
					const uint64_t mutex_id = id % (MUTEX_COUNT);
					lock_write(mutex_id);
					Node* prev = NULL;
					Node* node = buckets[id];
					while(node) {
						if(node->key == key)
							break;
						prev = node;
						node = node->next;
					}
					if(node == NULL) {
						unlock_write(mutex_id);
						return;
					}
					if(prev)
						prev->next = node->next;
					else
						buckets[id] = node->next;
					unlock_write(mutex_id);
					allocator->free(node);
					return;
				}
				
				template<typename X>
				inline void __debug_foreach(X f) {
					for(uint64_t i=0; i<size; ++i) {
						Node* node = buckets[i];
						while(node) {
							f(node->key, node->value);
							node = node->next;
						}
					}
				}
				
			private:
				
				inline void lock_write(uint64_t id) {
					if constexpr (MUTEX_COUNT > 0) mutexes[id].lock();
				}
				inline void unlock_write(uint64_t id) {
					if constexpr (MUTEX_COUNT > 0) mutexes[id].unlock();
				}
				inline void lock_read(uint64_t id) {
					if constexpr (MUTEX_COUNT > 0) mutexes[id].lock();
				}
				inline void unlock_read(uint64_t id) {
					if constexpr (MUTEX_COUNT > 0) mutexes[id].unlock();
				}
				
				Node** buckets;
				uint64_t size;
				Allocator* allocator;
				concurrent::mpmc::mutex<64> mutexes[MUTEX_COUNT];
			};
		};
	}
}

#endif

