// Copyright (C) 2021-2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

#ifndef CONCURRENT_SPSC_RINGBUFFER_HPP
#define CONCURRENT_SPSC_RINGBUFFER_HPP

#include <atomic>
#include <cstdlib>
#include <bit>

namespace concurrent {
	namespace spsc {
		template<typename T, size_t size>
		class ringbuffer {
		public:
			static_assert(std::has_single_bit(size),
					"size in ringbuffer must be non-zero a power of 2");
			static_assert(size >= 4, "size in ringbuffer must be at least 2");
			static_assert(size < (((size_t)1)<<(sizeof(size_t)*8-2)),
					"size in ringbuffer must not be near size_t limits "
					"(logarithmically)");
			
			inline const static size_t mask = size-1;
			
			ringbuffer() : _head(0), _tail(0) {}
			ringbuffer(ringbuffer&&) = delete;
			ringbuffer(const ringbuffer&) = delete;
			~ringbuffer() {}
			
			ringbuffer& operator=(ringbuffer&&) = delete;
			ringbuffer& operator=(const ringbuffer&) = delete;
			
			inline bool is_empty() const { return _head == _tail; }
			inline bool is_not_empty() const { return !is_empty(); }
			inline bool is_not_full() const { return !is_full(); }
			inline bool is_full() const { return _head-size >= _tail; }
			
			
			inline T& head() { return _data[_head&mask]; }
			inline bool push(const T& value) {
				if(is_full())
					return false;
				head() = value;
				push();
				return true;
			}
			inline bool push(T&& value) {
				if(is_full())
					return false;
				head() = value;
				push();
				return true;
			}
			// Require is_full() == false
			inline void push() { ++_head;}
			
			
			inline T& tail() { return _data[_tail&mask]; }
			inline bool pop(T& value) {
				if(is_empty())
					return false;
				value = tail();
				pop();
				return true;
			}
			// Require is_empty() == false
			inline void pop() { ++_tail; }
			
			
			void clear() {
				size_t value = _tail, h = _head;
				while(_tail.compare_exchange_weak(h, value)) {
					h = _head;
					value = _tail;
				}
			}
			
			T* data() { return _data; }
			
		private:
			std::atomic<size_t> _head, _tail;
			T _data[size];
		};
	}
}

#endif

