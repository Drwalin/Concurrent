/*
 *  Concurrent primitive data structures.
 *  Copyright (C) 2024 Marek Zalewski aka Drwalin
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

#ifndef CONCURRENT_THREADED_POLL_HPP
#define CONCURRENT_THREADED_POLL_HPP

#include <cstdint>

#include <atomic>
#include <mutex>

#include "node_stack.hpp"

namespace nonconcurrent
{
template<size_t BYTES, size_t OBJECTS_PER_BUCKET>
class thread_local_pool;
}

namespace concurrent
{
template<size_t BYTES>
struct _byte_array : public concurrent::node<_byte_array<BYTES>>
{
};

template<size_t BYTES>
class buckets_pool
{
public:
	static_assert(BYTES >= sizeof(void*)*2, "threaded_pool requires at least sizeof(void*)*2 bytes per object");
	
	using byte_array = _byte_array<BYTES>;
	using node_stack = nonconcurrent::node_stack<byte_array>;
	
	buckets_pool(size_t max_buckets) : max_buckets(max_buckets) {
		buckets = new std::atomic<byte_array*>[max_buckets];
		sizes = new size_t[max_buckets];
		for (int i=0; i<max_buckets; ++i) {
			buckets[i] = NULL;
			sizes[i] = 0;
		}
	}
	~buckets_pool() {
		free_all();
		delete[] buckets;
		delete[] sizes;
		buckets = NULL;
		sizes = NULL;
	}
	
	void release_bucket(node_stack &bucket, size_t count) {
		bucket_releases_count++;
		if (buckets_count.load() < max_buckets) {
			std::lock_guard lock(mutex);
			if (buckets_count.load() < max_buckets) {
				_internal_release_bucket(bucket, count);
				return;
			}
		}
		size_t c = 0;
		while(bucket.empty() == false) {
			free(bucket.pop());
			++c;
		}
		sum_object_release += count;
		system_frees_count += c;
	}
	
	byte_array *acquire_bucket(size_t *count) {
		if (buckets_count.load() > 0) {
			std::lock_guard lock(mutex);
			if (buckets_count.load() > 0) {
				bucket_acquisitions_count++;
				return _internal_acquire_bucket(count);
			}
		}
		*count = 1;
		byte_array *ptr = (byte_array *)malloc(BYTES);
		++system_allocations_count;
		++sum_object_acquisition;
		ptr->__m_next = NULL;
		return ptr;
	}
	
	uint64_t estimate_system_allocations() const {
		return system_allocations_count.load();
	}
	
	uint64_t estimate_system_frees() const {
		return system_frees_count.load();
	}
	
	uint64_t count_bucket_acquisitions() const {
		return bucket_acquisitions_count.load();
	}
	
	uint64_t count_bucket_releases() const {
		return bucket_releases_count.load();
	}
	
	uint64_t current_memory_resident_objects() const {
		uint64_t frees = system_frees_count.load();
		return system_allocations_count.load() - frees;
	}
	
	uint64_t current_memory_resident() const {
		return current_memory_resident_objects() * BYTES;
	}
	
	uint64_t count_objects_in_global_pool() const {
		return objects_in_glob;
		uint64_t sum = 0;
		for (int i=0; i<buckets_count; ++i) {
			sum += sizes[i];
		}
		return sum;
	}
	
	static size_t single_block_size() {
		return BYTES;
	}
	
	void free_all() {
		std::lock_guard lock(mutex);
		for (int i=0; i<max_buckets; ++i) {
			node_stack b;
			if (buckets[i] != NULL) {
				b.push_all(buckets[i]);
			}
			buckets[i] = NULL;
			sizes[i] = 0;
			while (!b.empty()) {
				free(b.pop());
				system_frees_count++;
				objects_in_glob--;
			}
		}
	}
	
	std::mutex mutex2;
	template<size_t S>
	nonconcurrent::node_stack<nonconcurrent::thread_local_pool<BYTES, S>> &mod_tls_pool(nonconcurrent::thread_local_pool<BYTES, S> *tls_pool, bool adding) {
		static nonconcurrent::node_stack<nonconcurrent::thread_local_pool<BYTES, S>> stack;
		std::lock_guard lock(mutex2);
		if (tls_pool != NULL) {
			if (adding) {
				tls_pool->__m_next = NULL;
				stack.push(tls_pool);
			} else {
				nonconcurrent::node_stack<nonconcurrent::thread_local_pool<BYTES, S>> tmp;
				while (!stack.empty()) {
					auto p = stack.pop();
					if (p == tls_pool) {
						break;
					} else {
						tmp.push(p);
					}
				}
				if (tmp.empty() == false) {
					stack.push_all(tmp.pop_all());
				}
			}
		}
		
		return stack;
	}
	
private:
	void _internal_release_bucket(node_stack &bucket, size_t count) {
		size_t id = buckets_count.load();
		buckets[id] = bucket.pop_all();
		sizes[id] = count;
		objects_in_glob += count;
		buckets_count++;
		sum_object_release += count;
	}
	
	byte_array *_internal_acquire_bucket(size_t *count) {
		buckets_count--;
		*count = sizes[buckets_count];
		sizes[buckets_count] = 0;
		byte_array *ret = buckets[buckets_count];
		buckets[buckets_count] = NULL;
		objects_in_glob -= *count;
		sum_object_acquisition += *count;
		return ret;
	}
	
private:
	
	std::mutex mutex;
	const size_t max_buckets;
	std::atomic<byte_array*> *buckets;
	size_t *sizes;
	std::atomic<uint64_t> system_allocations_count = 0;
	std::atomic<uint64_t> system_frees_count = 0;
	std::atomic<uint64_t> bucket_acquisitions_count = 0;
	std::atomic<uint64_t> bucket_releases_count = 0;
	std::atomic<size_t> buckets_count = 0;
	std::atomic<uint64_t> objects_in_glob = 0;
	
public:
	std::atomic<uint64_t> local_sum_acquisition = 0;
	std::atomic<uint64_t> local_sum_release = 0;
	
	std::atomic<uint64_t> sum_object_acquisition = 0;
	std::atomic<uint64_t> sum_object_release = 0;
};
}

namespace nonconcurrent
{
template<size_t BYTES, size_t OBJECTS_PER_BUCKET>
class thread_local_pool : public concurrent::node<thread_local_pool<BYTES, OBJECTS_PER_BUCKET>>
{
public:
	
	bool resident = true;
	
	thread_local_pool(concurrent::buckets_pool<BYTES> *buckets_pool) {
		this->buckets_pool = buckets_pool;
		buckets_pool->mod_tls_pool(this, true);
	}
	~thread_local_pool() {
		buckets_pool->mod_tls_pool(this, false);
		release_buckets_to_global();
		resident = false;
	}
	
	void release_buckets_to_global() {
		for (int i=0; i<2; ++i) {
			_internal_swap();
			if (size[1] > 0) {
				_internal_release_one_bucket();
			}
		}
	}
	
	using byte_array = concurrent::_byte_array<BYTES>;
	
	template<typename T, typename... Args>
	T *acquire(Args... args) {
		if (resident == false) {
			return new(malloc(BYTES)) T(std::move(args)...);
		}
		static_assert(sizeof(T) <= BYTES);
		buckets_pool->local_sum_acquisition++;
		if (size[0] == 0) {
			if (size[1] == 0) {
				_internal_acquire_one_bucket();
			} else {
				_internal_swap();
			}
		}
		size[0]--;
		byte_array *ptr = buckets[0].pop();
		return new(ptr) T(std::move(args)...);
	}
	
	template<typename T>
	void release(T *ptr) {
		if (resident == false) {
			ptr->~T();
			free(ptr);
			return;
		}
		buckets_pool->local_sum_release++;
		ptr->~T();
		if (size[1] >= OBJECTS_PER_BUCKET) {
			if (size[0] >= OBJECTS_PER_BUCKET) {
				_internal_release_one_bucket();
			} else {
				_internal_swap();
			}
		}
		size[1]++;
		ptr->__m_next = NULL;
		buckets[1].push((byte_array*)ptr);
	}
	
private:
	void _internal_swap() {
		std::swap(size[0], size[1]);
		std::swap(buckets[0], buckets[1]);
	}
	
	void _internal_release_one_bucket() {
		buckets_pool->release_bucket(buckets[1], size[1]);
		size[1] = 0;
	}
	
	void _internal_acquire_one_bucket() {
		buckets[0].push_all(buckets_pool->acquire_bucket(&(size[0])));
	}
	
private:
	node_stack<byte_array> buckets[2];
	size_t size[2] = {0, 0};
	
	concurrent::buckets_pool<BYTES> *buckets_pool;
};
}

#endif
