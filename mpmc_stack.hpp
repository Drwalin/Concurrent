// Copyright (C) 2024-2025 Marek Zalewski aka Drwalin
//
// This file is part of Concurrent project under MIT License
// You should have received a copy of the MIT License along with this program.

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
