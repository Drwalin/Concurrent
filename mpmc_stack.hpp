/*
 *  Concurrent queue, stack, stackqueue and object pool.
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

#ifndef POOL_MPMC_STACK_HPP
#define POOL_MPMC_STACK_HPP

#include <mutex>

#include "mpsc_stack.hpp"

namespace concurrent
{
namespace mpmc
{
template<typename T>
class mpmc_stack {
public:
	
	mpmc_stack(mpmc_stack&&) = delete;
	mpmc_stack(const mpmc_stack&) = default;
	mpmc_stack& operator =(const mpmc_stack&) = delete;
	mpmc_stack& operator =(mpmc_stack&&) = default;
	
	mpmc_stack() {}
	~mpmc_stack() {}
	
	inline T* pop() {
		std::lock_guard<std::mutex> lock(mutex);
		for(;;) {
			T* value = stack.first;
			if(value == NULL)
				return NULL;
			T* next = value->__m_next;
			if(stack.first.compare_exchange_strong(value, next)) {
				return value;
			}
		}
		return NULL;
	}
	
	//	safe to call with concurrent push, but without concurrent pop
	inline T* pop_sequentially() {
		return stack.pop();
	}
	
	//	pop whole stack at once, caller must handle returned list
	inline T* pop_all() {
		std::lock_guard<std::mutex> lock(mutex);
		return stack.pop_all();
	}
	
	//	safe to call without concurrent pop
	inline T* pop_unsafe() {
		return stack.pop();
	}
	
	//	safe to call without concurrent pop
	//	pop whole stack at once, caller must handle returned list
	inline T* pop_all_unsafe() {
		std::lock_guard<std::mutex> lock(mutex);
		return stack.pop_all();
	}
	
	inline void push(T* new_node) {
		stack.push(new_node);
	}
	
	//	safe to call without concurrent push nor pop
	inline void push_sequentially(T* new_node) {
		stack.push_sequentially(new_node);
	}
	
	inline void push_all(T* _first) {
		stack.push_all(_first);
	}
	
	//	safe to call without concurrent push nor pop
	inline void push_all_unsafe(T* _first) {
		stack.push_all_unsafe(_first);
	}
	
	inline void reverse() {
		stack.reverse();
	}
	
	//	safe to call without concurrent push nor pop
	inline void reverse_unsafe() {
		stack.reverse_unsafe();
	}
	
private:
	
	std::mutex mutex;
	mpsc::stack<T> stack;
};
}
}

#endif
