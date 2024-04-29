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
#include <new>

#include "node_stack.hpp"

namespace concurrent
{
template<size_t BYTES>
struct _byte_array : public concurrent::node<_byte_array<BYTES>>
{
};
}

namespace concurrent
{
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
	}
	~buckets_pool() {
		delete buckets;
		delete sizes;
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
		while(bucket.empty() == false) {
			::operator delete(bucket.pop());
			++system_frees_count;
		}
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
		byte_array *ptr = (byte_array *)::operator new(BYTES);
		++system_allocations_count;
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
	
	static size_t single_block_size() {
		return BYTES;
	}
	
private:
	void _internal_release_bucket(node_stack &bucket, size_t count) {
		size_t id = buckets_count.load();
		buckets[id] = bucket.pop_all();
		sizes[id] = count;
		buckets_count++;
	}
	
	byte_array *_internal_acquire_bucket(size_t *count) {
		buckets_count--;
		*count = sizes[buckets_count];
		return buckets[buckets_count];
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
	
public:
	std::atomic<uint64_t> local_sum_acquisition = 0;
	std::atomic<uint64_t> local_sum_release = 0;
};
}

namespace nonconcurrent
{
template<size_t BYTES, size_t OBJECTS_PER_BUCKET>
class thread_local_pool
{
public:
	
	thread_local_pool(concurrent::buckets_pool<BYTES> *buckets_pool) {
		this->buckets_pool = buckets_pool;
	}
	~thread_local_pool() {
		
	}
	
	using byte_array = concurrent::_byte_array<BYTES>;
	
	template<typename T, typename... Args>
	T *acquire(Args... args) {
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
		byte_array *ar = buckets[0].pop();
		void *ptr = (void *)ar;
		return new(ptr) T(std::move(args)...);
	}
	
	template<typename T>
	void release(T *ptr) {
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
